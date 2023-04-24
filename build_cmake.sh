#!/bin/sh

conan install . --output-folder=build_cmake --build=missing

if [[ ! -d "./build_cmake" ]]
then
    echo "Conan din't run properly"
	exit 1
fi

# Conan might fail and still create the the build_cmake folder, 
#	but at least we make sure not to pollute the root folder with build files

if [[ ! -d output ]]
then
    mkdir output
fi

echo '**' > ./build_cmake/.gitignore
echo '**' > ./output/.gitignore


cd build_cmake 

cmake .. -DCMAKE_TOOLCHAIN_FILE="./conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build .
cp ./bin/* ../output/
cd ..
