{
  lib,
  stdenv,
  cmake,
  pkg-config,
  hyprlang,
  hyprutils,
  qt6Packages,
  version ? "git",
}:
stdenv.mkDerivation {
  pname = "hyprqt6engine";
  inherit version;

  src = ../.;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    hyprlang
    hyprutils
    qt6Packages.qtbase
  ];

  dontWrapQtApps = true;

  cmakeFlags = lib.mapAttrsToList lib.cmakeFeature {
    PLUGINDIR = "${placeholder "out"}/lib/qt-6";
  };

  meta = {
    homepage = "https://github.com/hyprwm/hyprqt6engine";
    description = "Qt6 theme provider for Hyprland";
    license = lib.licenses.bsd3;
    platforms = lib.platforms.linux;
  };
}
