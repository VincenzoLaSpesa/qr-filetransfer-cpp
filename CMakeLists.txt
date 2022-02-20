cmake_minimum_required (VERSION 3.12)

project(qr-filetransfer_cpp VERSION LANGUAGES C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	MESSAGE(STATUS "No toolchain is defined, this could cause issues with vcpkg")
else()
	MESSAGE(STATUS "Using toolchain in ${CMAKE_TOOLCHAIN_FILE} ")
endif()

if(EXISTS "ConanWrapper.cmake")
	include("ConanWrapper.cmake")
else()
	MESSAGE( STATUS "Conan is not used")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(OBT_64 ON)
	MESSAGE( STATUS "Running on x64")
else()
	set(OBT_32 ON)
	MESSAGE( STATUS "Running on x86")
endif()

MESSAGE( STATUS "Looking for libs ... " )
  #restinio and its dependencies
  #find_package(unofficial-http-parser REQUIRED)
  find_package(fmt REQUIRED)
  find_package(asio REQUIRED)

  find_package(args REQUIRED)    
  find_package(httplib REQUIRED) # there is more than one lib called like this, see the Conan file for more info

  find_path(QRENCODE_INCLUDE_DIR NAMES qrencode.h)
  find_library(QRENCODE_LIBRARY_RELEASE qrencode)
  find_library(QRENCODE_LIBRARY_DEBUG qrencoded)
  set(QRENCODE_LIBRARIES optimized ${QRENCODE_LIBRARY_RELEASE} debug ${QRENCODE_LIBRARY_DEBUG})	
MESSAGE( STATUS "All libs found. " )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_INSTALL_DIR "bin")

add_subdirectory(src)




