project('qr-filetransfer_cpp', 'cpp')
src = ['./src/main.cpp', './src/Server.cpp', './src/Server.h', './src/htmldata.hpp', './src/CustomLogger.h', './src/Util.h']

# dependencies will be handled transparently by conan ( hopefully )
deps = [
	dependency('fmt', version : '>=0.0.1'),# i trust conan to install only one
	dependency('libqrencode', version : '>=0.0.1'),
	dependency('args', version : '>=0.0.1'),
	dependency('cpp-httplib', version : '>=0.0.1')
]

executable('qr-filetransfer', src, dependencies: deps, install : true, install_dir : './output/', cpp_args : '-std=c++17')


# IMPORTANT: for making Mason+Conan work on windows you need to install pkgconfiglite and ninja
#		choco install meson ninja pkgconfiglite
#		pip install conan
# imho is safer to install meson with choco and conan with pip.
