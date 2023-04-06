#if(Test-Path 'build_cmake'){rm build_cmake -r -force}

conan install . --output-folder=build_cmake --build=missing --profile=./conan_profiles/vs2022_x64.profile
if(!(Test-Path 'build_cmake')){throw "Conan din't run properly"}
# Conan might fail and still create the the build_cmake folder, but at least we make sure not to pollute the root folder with build files

if(!(Test-Path '.\build_cmake\.gitignore')){
	New-Item ".\build_cmake\.gitignore" -ItemType File -Value '**'
}

if(!(Test-Path 'output')){
    mkdir output
    New-Item ".\output\.gitignore" -ItemType File -Value '**'
}

cd build_cmake 

$targets=@(
	"Visual Studio 17 2022",
	"Visual Studio 16 2019",
	"Visual Studio 15 2017",
	"Visual Studio 9 2008"
)
cmake .. -G $targets[0] -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
cmake --build . --config Release
cp .\bin\Release\* ..\output\

cd ..
