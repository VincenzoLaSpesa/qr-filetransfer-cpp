@echo off
pushd ..
set ABS_PATH=%CD%
popd

set TOOLCHAIN=D:\Codice\vcpkg\scripts\buildsystems\vcpkg.cmake
set TARGET1="Visual Studio 16 2019"
set TARGET2="Visual Studio 15 2017"
set TARGET3="Visual Studio 9 2008"
set CONAN_REVISIONS_ENABLED=1

@echo on
mkdir build_2019
cd build_2019
conan install .. --build=missing
cd ..

cmake -S . -B .\build_2019\ -G %TARGET1% -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONAN_TOOLCHAIN_FOLDER:STRING=build_2019
cmake --build .\build_2019 --config Debug 
