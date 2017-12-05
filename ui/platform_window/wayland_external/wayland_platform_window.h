// Copyright 2017 LG Electronics, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_PLATFORM_WINDOW_WAYLAND_EXTERNAL_WAYLAND_PLATFORM_WINDOW_H_
#define UI_PLATFORM_WINDOW_WAYLAND_EXTERNAL_WAYLAND_PLATFORM_WINDOW_H_

#include "base/strings/string16.h"
#include "ui/gfx/native_widget_types.h"

class SkPath;

namespace ui {
// ozone-wayland additions for platform window.
class WaylandPlatformWindow {
 public:
  enum PlatformWindowType {
    PLATFORM_WINDOW_UNKNOWN,
    PLATFORM_WINDOW_TYPE_TOOLTIP,
    PLATFORM_WINDOW_TYPE_POPUP,
    PLATFORM_WINDOW_TYPE_MENU,
    PLATFORM_WINDOW_TYPE_BUBBLE,
    PLATFORM_WINDOW_TYPE_WINDOW,
    PLATFORM_WINDOW_TYPE_WINDOW_FRAMELESS
  };

  virtual void InitPlatformWindow(PlatformWindowType type,
                                  gfx::AcceleratedWidget parent_window) {}
  virtual void SetWindowShape(const SkPath& path) {}
  virtual void SetOpacity(unsigned char opacity) {}

  // Asks the GPU process to send data of type |mime_type|.
  virtual void RequestDragData(const std::string& mime_type) {}
  virtual void RequestSelectionData(const std::string& mime_type) {}

  // Indicates to the drag source that the data will or will not be accepted
  // at the current (x, y) coordinates. Note that there is a harmless race here.
  // The browser process could decide to accept or reject the data based on
  // old (x, y) coordinates that have since been updated by a new DragMotion
  // event in the GPU process. This doesn't matter because the browser process
  // will promptly correct the matter by calling one of these functions again
  // when it receives the DragMotion event, and these functions are only used to
  // provide user feedback: they don't affect correctness.
  virtual void DragWillBeAccepted(uint32_t serial,
                                  const std::string& mime_type) {}
  virtual void DragWillBeRejected(uint32_t serial) {}
};

}  // namespace ui

#endif  // UI_PLATFORM_WINDOW_WAYLAND_EXTERNAL_WAYLAND_PLATFORM_WINDOW_H_
