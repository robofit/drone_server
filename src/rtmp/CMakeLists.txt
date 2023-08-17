cmake_minimum_required (VERSION 3.8)

project ("rtmp")

set(SOURCES
    src/${SOLUTION_NAME}/${PROJECT_NAME}/rtmp_session.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/rtmp_session_control_message.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/rtmp_session_command_message.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/rtmp_chunk_stream.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/rtmp_handshake.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/rtmp_stream.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/gop_cache.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/flv_demuxer.cpp

    src/${SOLUTION_NAME}/${PROJECT_NAME}/amf0/amf0.cpp
    src/${SOLUTION_NAME}/${PROJECT_NAME}/amf0/simple_buffer.cpp
)

set(INCLUDES

)

# Packages
#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
find_package(OpenSSL REQUIRED)
##


# Target
add_library(${PROJECT_NAME} STATIC
    ${SOURCES}
)
add_library(${SOLUTION_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  IMPORTED_LOCATION ${CMAKE_INSTALL_BINDIR}
  IMPORTED_IMPLIB ${CMAKE_INSTALL_LIBDIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    DDS::core
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src
    ${OPENSSL_INCLUDE_DIR}
)
##

# Install
install(DIRECTORY
    src/${SOLUTION_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "*.hpp"
)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
##

# Export
install(EXPORT ${PROJECT_NAME}Targets
   FILE ${PROJECT_NAME}Config.cmake
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
##

