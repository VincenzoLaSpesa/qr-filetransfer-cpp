#!/bin/sh
rm -rf conan_wrapper
mkdir output

conan install . --output-folder=conan_wrapper --build=missing
cd conan_wrapper
meson setup --native-file conan_meson_native.ini .. meson_build
cd meson_build
meson compile
meson install --destdir ../../output
