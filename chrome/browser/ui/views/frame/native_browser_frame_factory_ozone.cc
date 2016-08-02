// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/frame/native_browser_frame_factory.h"

#include "chrome/browser/ui/views/frame/browser_frame_mus.h"
#include "ui/aura/env.h"

// Added for ozone wayland external port
#if defined(OZONE_PLATFORM_WAYLAND_EXTERNAL)
#include "chrome/browser/ui/views/frame/desktop_browser_frame_aura.h"
#endif

NativeBrowserFrame* NativeBrowserFrameFactory::Create(
    BrowserFrame* browser_frame,
    BrowserView* browser_view) {
  if (aura::Env::GetInstance()->mode() == aura::Env::Mode::MUS)
    return new BrowserFrameMus(browser_frame, browser_view);

// Added for ozone wayland external port
// For chrome browser with ozone wayland port
// we should create DesktopBrowserFrameAura.
#if defined(OZONE_PLATFORM_WAYLAND_EXTERNAL)
  return new DesktopBrowserFrameAura(browser_frame, browser_view);
#endif
  NOTREACHED() << "For Ozone builds, only --mash launch is supported for now.";
  return nullptr;
}
