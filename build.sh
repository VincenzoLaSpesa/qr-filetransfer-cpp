TOOLCHAIN="put here a vcpkg toolchain, if you need it"
BUILD_FOLDER="build"

export CONAN_REVISIONS_ENABLED=1

# remove this if you are not behind some very intrusive proxy
conan remote remove conancenter
conan remote add conancenter "https://center.conan.io" False

mkdir ${BUILD_FOLDER}
cd ${BUILD_FOLDER}
conan install .. --build=missing
conan install .. --build=missing --profile=../conan_profiles/gcc10_x64_linux.profile
cd ..

ABS_PATH=$(pwd)
cd src
git clone https://github.com/VincenzoLaSpesa/picohash
cd picohash
git pull
cd ${ABS_PATH}


cmake -S . -B ./$BUILD_FOLDER/ -G Ninja -DCONAN_TOOLCHAIN_FOLDER:STRING=$BUILD_FOLDER -DCMAKE_EXPORT_COMPILE_COMMANDS=True
mkdir .vscode
cp -r ./vscode_template/* ./.vscode/

#cmake -S . -B ./$BUILD_FOLDER/ -DCONAN_TOOLCHAIN_FOLDER:STRING=$BUILD_FOLDER
cmake --build ./$BUILD_FOLDER
 
#mkdir ${BUILD_FOLDER}_eclipse
#cd ${BUILD_FOLDER}_eclipse
#conan install .. --build=missing
#conan install .. --build=missing --profile=../conan_profiles/gcc10_x64_linux.profile
#cd ..
#cmake -S . -B ./${BUILD_FOLDER}_eclipse/ -G"Eclipse CDT4 - Ninja" -DCONAN_TOOLCHAIN_FOLDER:STRING=${BUILD_FOLDER}_eclipse

#mkdir ${BUILD_FOLDER}_makefile
#cd ${BUILD_FOLDER}_makefile
#conan install .. --build=missing
#conan install .. --build=missing --profile=../conan_profiles/gcc10_x64_linux.profile
#cd ..
#cmake -S . -B ./${BUILD_FOLDER}_makefile/ -DCONAN_TOOLCHAIN_FOLDER:STRING=${BUILD_FOLDER}_makefile
