#!/bin/sh

conan install . --output-folder=build_meson --build=missing

if [[ ! -f build_meson ]]
then
    echo "Conan din't run properly"
	exit 1
fi
# Conan might fail and still create the the build_meson folder, 
#	but at least we make sure not to pollute the root folder with build files

if [[ ! -f output ]]
then
    mkdir output
fi

echo '**' > ./build_meson/.gitignore
echo '**' > ./output/.gitignore


cd build_meson 

meson setup --native-file conan_meson_native.ini .. build_meson_ninja
cd build_meson_ninja
meson install --destdir ../../
cd ..
cd ..
