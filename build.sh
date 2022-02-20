TOOLCHAIN="put here a vcpkg toolchain, if you need it"
BUILD_FOLDER="build_posix"

export CONAN_REVISIONS_ENABLED=1

# remove this if you are not behind some very intrusive proxy
conan remote remove conancenter
conan remote add conancenter "https://center.conan.io" False

mkdir ${BUILD_FOLDER}
cd ${BUILD_FOLDER}
conan install .. --build=missing
cd ..

cmake -S . -B ./$BUILD_FOLDER/ -G Ninja -DCONAN_TOOLCHAIN_FOLDER:STRING=$BUILD_FOLDER
#cmake -S . -B ./$BUILD_FOLDER/ -DCONAN_TOOLCHAIN_FOLDER:STRING=$BUILD_FOLDER
cmake --build ./$BUILD_FOLDER
# Based on https://github.com/VincenzoLaSpesa/MinimalBuildTemplate 'Thu Feb 17 12:22:21 2022 +0100'
 
