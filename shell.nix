with import <nixpkgs> {};
stdenv.mkDerivation rec {
    pname = "keyd";
    version = "1.1.2";
    src = null;
    buildInputs = [ pkgconfig libudev ];
    meta = {
      license = lib.licenses.mit;
      homepage = "https://github.com/rvaiya/keyd";
      description = "Remap keys using kernel-level input primitives (evdev, uinput).";
    };
    platforms = lib.platforms.all;
}
