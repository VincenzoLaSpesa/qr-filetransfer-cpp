add_executable(QrFiletransfer main.cpp Server.cpp Server.h htmldata.hpp ConnectionListener.h CustomLogger.h)
set_target_properties(QrFiletransfer PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Debug")
target_link_libraries(QrFiletransfer PRIVATE restinio::restinio) 
target_link_libraries(QrFiletransfer PRIVATE fmt::fmt-header-only)
target_link_libraries(QrFiletransfer PRIVATE taywee::args)
target_link_libraries(QrFiletransfer PRIVATE asio::asio)

target_include_directories(QrFiletransfer PRIVATE ${QRENCODE_INCLUDE_DIR})
target_link_libraries(QrFiletransfer PRIVATE ${QRENCODE_LIBRARIES})

  install(
    TARGETS
	QrFiletransfer
    RUNTIME DESTINATION ${PROJECT_INSTALL_DIR})