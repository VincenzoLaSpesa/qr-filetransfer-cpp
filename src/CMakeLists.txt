add_executable(QrFiletransfer main.cpp Server.cpp Server.h htmldata.hpp CustomLogger.h Util.h)
set_target_properties(QrFiletransfer PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Debug")
target_link_libraries(QrFiletransfer PRIVATE fmt::fmt)
target_link_libraries(QrFiletransfer PRIVATE taywee::args)
target_link_libraries(QrFiletransfer PRIVATE Threads::Threads)
#target_link_libraries(QrFiletransfer PRIVATE httplib) not needed for headers-only version

target_include_directories(QrFiletransfer PRIVATE ${QRENCODE_INCLUDE_DIR})
target_link_libraries(QrFiletransfer PRIVATE ${QRENCODE_LIBRARIES})
target_compile_features(QrFiletransfer PRIVATE cxx_std_17)


  install(
    TARGETS
	QrFiletransfer
    RUNTIME DESTINATION ${PROJECT_INSTALL_DIR})