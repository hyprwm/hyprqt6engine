{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";

    hyprutils = {
      url = "github:hyprwm/hyprutils";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };

    hyprlang = {
      url = "github:hyprwm/hyprlang";
      inputs = {
        hyprutils.follows = "hyprutils";
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };
  };

  outputs = inputs @ {
    self,
    nixpkgs,
    systems,
    ...
  }: let
    inherit (nixpkgs) lib;
    eachSystem = lib.genAttrs (import systems);
    pkgsFor = eachSystem (
      system:
        import nixpkgs {
          localSystem.system = system;
          overlays = with self.overlays; [
            hyprqt6engine
          ];
        }
    );
  in {
    overlays = import ./nix/overlays.nix {inherit self lib inputs;};

    packages = eachSystem (system: {
      default = self.packages.${system}.hyprqt6engine;
      inherit (pkgsFor.${system}) hyprqt6engine;
    });

    checks = eachSystem (system: self.packages.${system});

    formatter = eachSystem (system: pkgsFor.${system}.alejandra);
  };
}
