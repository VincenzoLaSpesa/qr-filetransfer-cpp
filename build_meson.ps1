#if(Test-Path 'build_meson'){rm build_meson -r -force}

conan install . --output-folder=build_meson --build=missing
if(!(Test-Path 'build_meson')){throw "Conan din't run properly"}
# Conan might fail and still create the the build_meson folder, but at least we make sure not to pollute the root folder with build files

New-Item ".\build_meson\.gitignore" -ItemType File -Value '**'

if(!(Test-Path 'output')){
    mkdir output
    New-Item ".\output\.gitignore" -ItemType File -Value '**'
}

cd build_meson 

meson setup --native-file conan_meson_native.ini .. build_meson_ninja
cd build_meson_ninja
meson install --destdir ..\..\output
cd ..

meson setup --native-file conan_meson_native.ini .. build_meson_vs --backend vs
cd ..
