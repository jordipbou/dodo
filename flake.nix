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
				];
				env.CMAKE_PREFIX_PATH = "${pkgs.sdl3.dev}";
			};
		};
}
