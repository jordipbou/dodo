{
	description = "Development shell";
	
	inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
	
	outputs = { self, nixpkgs }:
		let
			system = "x86_64-linux";
			pkgs = import nixpkgs {
			  inherit system;
			};
		in {
			devShells.${system}.default = pkgs.mkShell {
				packages = with pkgs; [
					gcc
					cmake
					pkg-config
					sdl3
					# X11 development libraries required by SDL3
					xorg.libX11
					xorg.libxcb
					xorg.libXext
					xorg.libXrandr
					xorg.libXi
					xorg.libXcursor
					xorg.libXfixes
					xorg.libXrender
					xorg.libXdamage
					xorg.libXcomposite
					xorg.libXinerama
					# Wayland development libraries required by SDL3
					libxkbcommon
					wayland
					wayland-protocols
					libdecor
				];
			};
		};
}
