// Copyright 2017 LG Electronics, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_PLATFORM_WINDOW_WAYLAND_EXTERNAL_WAYLAND_PLATFORM_WINDOW_DELEGATE_H_
#define UI_PLATFORM_WINDOW_WAYLAND_EXTERNAL_WAYLAND_PLATFORM_WINDOW_DELEGATE_H_

#include <vector>

namespace ui {
// ozone-wayland additions for platform window delegate.
class WaylandPlatformWindowDelegate {
 public:
  virtual void OnDragEnter(unsigned windowhandle,
                           float x,
                           float y,
                           const std::vector<std::string>& mime_types,
                           uint32_t serial) { }

  virtual void OnDragDataReceived(int fd) { }

  virtual void OnDragLeave() { }

  virtual void OnDragMotion(float x,
                            float y,
                            uint32_t time) { }

  virtual void OnDragDrop() { }

};

}  // namespace ui

#endif  // UI_PLATFORM_WINDOW_WAYLAND_EXTERNAL_WAYLAND_PLATFORM_WINDOW_DELEGATE_H_
