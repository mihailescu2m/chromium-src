// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/gl_context_egl.h"

#include <memory>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"

#if defined(USE_X11)
// Must be included before khronos headers or they will pollute the
// global scope with X11 macros.
#include "ui/gfx/x/x11.h"
#endif

#include "third_party/khronos/EGL/egl.h"
#include "third_party/khronos/EGL/eglext.h"
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_surface_egl.h"


#ifndef EGL_CHROMIUM_create_context_bind_generates_resource
#define EGL_CHROMIUM_create_context_bind_generates_resource 1
#define EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM 0x3AAD
#endif /* EGL_CHROMIUM_create_context_bind_generates_resource */

#ifndef EGL_ANGLE_create_context_webgl_compatibility
#define EGL_ANGLE_create_context_webgl_compatibility 1
#define EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE 0x3AAC
#endif /* EGL_ANGLE_create_context_webgl_compatibility */

#ifndef EGL_ANGLE_display_texture_share_group
#define EGL_ANGLE_display_texture_share_group 1
#define EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE 0x3AAF
#endif /* EGL_ANGLE_display_texture_share_group */

#ifndef EGL_ANGLE_create_context_client_arrays
#define EGL_ANGLE_create_context_client_arrays 1
#define EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE 0x3452
#endif /* EGL_ANGLE_create_context_client_arrays */

#ifndef EGL_ANGLE_robust_resource_initialization
#define EGL_ANGLE_robust_resource_initialization 1
#define EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE 0x3453
#endif /* EGL_ANGLE_display_robust_resource_initialization */

#ifndef EGL_CONTEXT_PRIORITY_LEVEL_IMG
#define EGL_CONTEXT_PRIORITY_LEVEL_IMG 0x3100
#define EGL_CONTEXT_PRIORITY_HIGH_IMG 0x3101
#define EGL_CONTEXT_PRIORITY_MEDIUM_IMG 0x3102
#define EGL_CONTEXT_PRIORITY_LOW_IMG 0x3103
#endif /* EGL_CONTEXT_PRIORITY_LEVEL */

using ui::GetLastEGLErrorString;

namespace gl {

GLContextEGL::GLContextEGL(GLShareGroup* share_group)
    : GLContextReal(share_group),
      context_(nullptr),
      display_(nullptr),
      config_(nullptr),
      unbind_fbo_on_makecurrent_(false) {}

bool GLContextEGL::Initialize(GLSurface* compatible_surface,
                              const GLContextAttribs& attribs) {
  DCHECK(compatible_surface);
  DCHECK(!context_);

  display_ = compatible_surface->GetDisplay();
  config_ = compatible_surface->GetConfig();

  EGLint config_renderable_type = 0;
  if (!eglGetConfigAttrib(display_, config_, EGL_RENDERABLE_TYPE,
                          &config_renderable_type)) {
    LOG(ERROR) << "eglGetConfigAttrib failed with error "
               << GetLastEGLErrorString();
    return false;
  }

  EGLint context_client_major_version = attribs.client_major_es_version;
  EGLint context_client_minor_version = attribs.client_minor_es_version;

  // If the requested context is ES3 but the config cannot support ES3, request
  // ES2 instead.
  if ((config_renderable_type & EGL_OPENGL_ES3_BIT) == 0 &&
      context_client_major_version >= 3) {
    context_client_major_version = 2;
    context_client_minor_version = 0;
  }

  std::vector<EGLint> context_attributes;

  // EGL_KHR_create_context allows requesting both a major and minor context
  // version
  if (GLSurfaceEGL::HasEGLExtension("EGL_KHR_create_context")) {
    context_attributes.push_back(EGL_CONTEXT_MAJOR_VERSION);
    context_attributes.push_back(context_client_major_version);

    context_attributes.push_back(EGL_CONTEXT_MINOR_VERSION);
    context_attributes.push_back(context_client_minor_version);
  } else {
    context_attributes.push_back(EGL_CONTEXT_CLIENT_VERSION);
    context_attributes.push_back(context_client_major_version);

    // Can only request 2.0 or 3.0 contexts without the EGL_KHR_create_context
    // extension, DCHECK to make sure we update the code to support devices
    // without this extension
    DCHECK(context_client_minor_version == 0);
  }

  if (GLSurfaceEGL::IsCreateContextRobustnessSupported()) {
    DVLOG(1) << "EGL_EXT_create_context_robustness supported.";
    context_attributes.push_back(EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT);
    context_attributes.push_back(attribs.robust_buffer_access ? EGL_TRUE
                                                              : EGL_FALSE);
    context_attributes.push_back(
        EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT);
    context_attributes.push_back(EGL_LOSE_CONTEXT_ON_RESET_EXT);
  } else {
    // At some point we should require the presence of the robustness
    // extension and remove this code path.
    DVLOG(1) << "EGL_EXT_create_context_robustness NOT supported.";
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    LOG(ERROR) << "eglBindApi failed with error "
               << GetLastEGLErrorString();
    return false;
  }

  if (GLSurfaceEGL::IsCreateContextBindGeneratesResourceSupported()) {
    context_attributes.push_back(EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM);
    context_attributes.push_back(attribs.bind_generates_resource ? EGL_TRUE
                                                                 : EGL_FALSE);
  } else {
    DCHECK(attribs.bind_generates_resource);
  }

  if (GLSurfaceEGL::IsCreateContextWebGLCompatabilitySupported()) {
    context_attributes.push_back(EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE);
    context_attributes.push_back(
        attribs.webgl_compatibility_context ? EGL_TRUE : EGL_FALSE);
  } else {
    DCHECK(!attribs.webgl_compatibility_context);
  }

  if (GLSurfaceEGL::IsEGLContextPrioritySupported()) {
    // Medium priority is the default, only set the attribute if
    // a different priority is requested.
    if (attribs.context_priority == ContextPriorityLow) {
      DVLOG(1) << __FUNCTION__ << ": setting ContextPriorityLow";
      context_attributes.push_back(EGL_CONTEXT_PRIORITY_LEVEL_IMG);
      context_attributes.push_back(EGL_CONTEXT_PRIORITY_LOW_IMG);
    } else if (attribs.context_priority == ContextPriorityHigh) {
      DVLOG(1) << __FUNCTION__ << ": setting ContextPriorityHigh";
      context_attributes.push_back(EGL_CONTEXT_PRIORITY_LEVEL_IMG);
      context_attributes.push_back(EGL_CONTEXT_PRIORITY_HIGH_IMG);
    }
  }

  if (GLSurfaceEGL::IsDisplayTextureShareGroupSupported()) {
    context_attributes.push_back(EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE);
    context_attributes.push_back(
        attribs.global_texture_share_group ? EGL_TRUE : EGL_FALSE);
  } else {
    DCHECK(!attribs.global_texture_share_group);
  }

  if (GLSurfaceEGL::IsCreateContextClientArraysSupported()) {
    // Disable client arrays if the context supports it
    context_attributes.push_back(EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE);
    context_attributes.push_back(EGL_FALSE);
  }

  if (GLSurfaceEGL::IsRobustResourceInitSupported()) {
    context_attributes.push_back(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE);
    context_attributes.push_back(
        attribs.robust_resource_initialization ? EGL_TRUE : EGL_FALSE);
  } else {
    DCHECK(!attribs.robust_resource_initialization);
  }

  // Append final EGL_NONE to signal the context attributes are finished
  context_attributes.push_back(EGL_NONE);
  context_attributes.push_back(EGL_NONE);

  context_ = eglCreateContext(
      display_, config_, share_group() ? share_group()->GetHandle() : nullptr,
      context_attributes.data());

  if (!context_) {
    LOG(ERROR) << "eglCreateContext failed with error "
               << GetLastEGLErrorString();
    return false;
  }

  return true;
}

void GLContextEGL::Destroy() {
  if (context_) {
    if (!eglDestroyContext(display_, context_)) {
      LOG(ERROR) << "eglDestroyContext failed with error "
                 << GetLastEGLErrorString();
    }

    context_ = nullptr;
  }
}

bool GLContextEGL::MakeCurrent(GLSurface* surface) {
  DCHECK(context_);
  if (IsCurrent(surface))
      return true;

  ScopedReleaseCurrent release_current;
  TRACE_EVENT2("gpu", "GLContextEGL::MakeCurrent",
               "context", context_,
               "surface", surface);

  if (unbind_fbo_on_makecurrent_ && GetCurrent()) {
    glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
  }

  if (!eglMakeCurrent(display_,
                      surface->GetHandle(),
                      surface->GetHandle(),
                      context_)) {
    DVLOG(1) << "eglMakeCurrent failed with error "
             << GetLastEGLErrorString();
    return false;
  }
// Added for ozone-wayland port
// Fix crash when switching to console(VT) mode
// Buffer swapping should not be synchronized so that the GPU process
// is not blocked by waiting for a frame update from Weston.
#if defined(USE_OZONE) && defined(OZONE_PLATFORM_WAYLAND_EXTERNAL)
  if (!surface->IsOffscreen())
    eglSwapInterval(display_, 0);
#endif

  // Set this as soon as the context is current, since we might call into GL.
  BindGLApi();

  SetCurrent(surface);
  InitializeDynamicBindings();

  if (!surface->OnMakeCurrent(this)) {
    LOG(ERROR) << "Could not make current.";
    return false;
  }

  release_current.Cancel();
  return true;
}

void GLContextEGL::SetUnbindFboOnMakeCurrent() {
  unbind_fbo_on_makecurrent_ = true;
}

void GLContextEGL::ReleaseCurrent(GLSurface* surface) {
  if (!IsCurrent(surface))
    return;

  if (unbind_fbo_on_makecurrent_)
    glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

  SetCurrent(nullptr);
  eglMakeCurrent(display_,
                 EGL_NO_SURFACE,
                 EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);
}

bool GLContextEGL::IsCurrent(GLSurface* surface) {
  DCHECK(context_);

  bool native_context_is_current = context_ == eglGetCurrentContext();

  // If our context is current then our notion of which GLContext is
  // current must be correct. On the other hand, third-party code
  // using OpenGL might change the current context.
  DCHECK(!native_context_is_current || (GetRealCurrent() == this));

  if (!native_context_is_current)
    return false;

  if (surface) {
    if (surface->GetHandle() != eglGetCurrentSurface(EGL_DRAW))
      return false;
  }

  return true;
}

void* GLContextEGL::GetHandle() {
  return context_;
}

void GLContextEGL::OnSetSwapInterval(int interval) {
  DCHECK(IsCurrent(nullptr) && GLSurface::GetCurrent());

  // This is a surfaceless context. eglSwapInterval doesn't take any effect in
  // this case and will just return EGL_BAD_SURFACE.
  if (GLSurface::GetCurrent()->IsSurfaceless())
    return;

  if (!eglSwapInterval(display_, interval)) {
    LOG(ERROR) << "eglSwapInterval failed with error "
               << GetLastEGLErrorString();
  }
}

bool GLContextEGL::WasAllocatedUsingRobustnessExtension() {
  return GLSurfaceEGL::IsCreateContextRobustnessSupported();
}

GLContextEGL::~GLContextEGL() {
  Destroy();
}

}  // namespace gl
