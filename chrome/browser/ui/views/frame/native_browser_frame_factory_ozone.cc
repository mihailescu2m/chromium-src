// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/frame/native_browser_frame_factory.h"

#include "chrome/browser/ui/views/frame/browser_frame_mus.h"
// Added for ozone wayland external port
#if defined(OZONE_PLATFORM_WAYLAND_EXTERNAL)
#include "chrome/browser/ui/views/frame/desktop_browser_frame_aura.h"
#endif
#include "services/service_manager/runner/common/client_util.h"

NativeBrowserFrame* NativeBrowserFrameFactory::Create(
    BrowserFrame* browser_frame,
    BrowserView* browser_view) {
  if (service_manager::ServiceManagerIsRemote())
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
