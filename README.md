# ![Logo](chrome/app/theme/chromium/product_logo_64.png) Chromium

# Introduction

This is a port to more recent Chromium and GN of the original [01.org](https://github.com/01org/ozone-wayland)
Ozone Wayland backend.

The ozone-wayland repository baseline used, was based on Chromium m51. This repository contains the adaptation
to Chromium m60, and has been provided by [LG Electronics](http://www.lg.com).

# IMPORTANT NOTE
This implementation is *legacy*. If you do not know what this is about, then very likely
you are not interested on this.

A new ozone wayland backend is being implemented in upstream Chromium. Check on
ui/ozone/platform/wayland.

If you are interested on making it run on a tradicional Wayland
desktop, then you may be interested on the
[downstream fork](https://github.com/Igalia/chromium/tree/ozone-wayland-dev) published by
[Igalia](http://www.igalia.com).

# Differences with [01.org](https://github.com/01org/ozone-wayland)

* Ported to GN
* Rebased on top of Chromium m60
* Reuses part of the work done upstream for the new Ozone Wayland backend.

# Build instructions

```
gn args out/Ozone --args="use_ozone=true ozone_platform_wayland_external=true ozone_auto_platforms=true ozone_platform_wayland=false ozone_platform_x11=false  use_xkbcommon=true"
ninja -C out/Ozone chrome 
./out/Ozone/chrome --ozone-platform=wayland
```

## License

Ozone-Wayland's code uses the BSD license (check the LICENSE file in the project).

# Original README.md message
Chromium is an open-source browser project that aims to build a safer, faster,
and more stable way for all users to experience the web.

The project's web site is https://www.chromium.org.

Documentation in the source is rooted in [docs/README.md](docs/README.md).

Learn how to [Get Around the Chromium Source Code Directory Structure
](https://www.chromium.org/developers/how-tos/getting-around-the-chrome-source-code).
