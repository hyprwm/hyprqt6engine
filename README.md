## hyprqt6engine

QT6 Theme Provider for Hyprland. Compatible with KDE, replaces qt6ct.

### Configuration

See the [wiki](https://wiki.hypr.land/Hypr-Ecosystem/hyprqt6engine/)

### Building

```sh
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr -S . -B ./build
cmake --build ./build --config Release --target all -j`nproc 2>/dev/null || getconf NPROCESSORS_CONF`
```

### Acknowledgements

A bunch of code is inspired by [qt6ct](https://www.opencode.net/trialuser/qt6ct)