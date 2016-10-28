// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/egl/gl_surface_wayland.h"

#include <wayland-egl.h>

#include <utility>

#include "third_party/khronos/EGL/egl.h"
#include "ui/ozone/common/egl_util.h"

namespace ozonewayland {

void EGLWindowDeleter::operator()(wl_egl_window* egl_window) {
  wl_egl_window_destroy(egl_window);
}

GLSurfaceWayland::GLSurfaceWayland(WaylandEglWindowPtr egl_window)
    : NativeViewGLSurfaceEGL(
          reinterpret_cast<EGLNativeWindowType>(egl_window.get())),
      egl_window_(std::move(egl_window)) {
  DCHECK(egl_window_);
}

bool GLSurfaceWayland::Resize(const gfx::Size& size,
                              float scale_factor,
                              bool has_alpha) {
  if (size_ == size)
    return true;
  wl_egl_window_resize(egl_window_.get(), size.width(), size.height(), 0, 0);
  size_ = size;
  return true;
}

EGLConfig GLSurfaceWayland::GetConfig() {
  if (!config_) {
    GLint config_attribs[] = {EGL_BUFFER_SIZE,
                              32,
                              EGL_ALPHA_SIZE,
                              8,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES2_BIT,
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_NONE};
    config_ = ui::ChooseEGLConfig(GetDisplay(), config_attribs);
  }
  return config_;
}

GLSurfaceWayland::~GLSurfaceWayland() {
  Destroy();
}

}  // namespace ozonewayland
