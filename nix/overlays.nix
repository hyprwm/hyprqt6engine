{
  lib,
  inputs,
  self,
}: let
  mkDate = longDate: (lib.concatStringsSep "-" [
    (builtins.substring 0 4 longDate)
    (builtins.substring 4 2 longDate)
    (builtins.substring 6 2 longDate)
  ]);

  version = lib.removeSuffix "\n" (builtins.readFile ../VERSION);
in {
  default = inputs.self.overlays.hyprqt6engine;

  hyprqt6engine = lib.composeManyExtensions [
    inputs.hyprlang.overlays.default
    inputs.hyprutils.overlays.default
    (final: prev: {
      hyprqt6engine = prev.callPackage ./default.nix {
        stdenv = prev.gcc15Stdenv;
        version =
          version
          + "+date="
          + (mkDate (inputs.self.lastModifiedDate or "19700101"))
          + "_"
          + (inputs.self.shortRev or "dirty");
        inherit (final) hyprlang hyprutils;
      };
    })
  ];
}
