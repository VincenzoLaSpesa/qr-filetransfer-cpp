add_executable(QrFiletransfer main.cpp Server.cpp Server.h htmldata.hpp Connections.h Connections.cpp CustomLogger.h)
set_target_properties(QrFiletransfer PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Debug")
target_link_libraries(QrFiletransfer PRIVATE restinio::restinio) 
target_link_libraries(QrFiletransfer PRIVATE fmt)
target_link_libraries(QrFiletransfer PRIVATE taywee::args)
target_link_libraries(QrFiletransfer PRIVATE asio::asio)

target_include_directories(QrFiletransfer PRIVATE ${QRENCODE_INCLUDE_DIR})
target_link_libraries(QrFiletransfer PRIVATE ${QRENCODE_LIBRARIES})
target_compile_features(QrFiletransfer PRIVATE cxx_std_17)


  install(
    TARGETS
	QrFiletransfer
    RUNTIME DESTINATION ${PROJECT_INSTALL_DIR})