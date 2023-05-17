cmake_minimum_required (VERSION 3.17)

project (core)

set(SOURCES
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/client.cpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/settings.cpp"

    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/encoder.cpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/decoder.cpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/copypipe.cpp"
)

set(INCLUDES
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/client.hpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/settings.hpp"

    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/encoder.hpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/decoder.hpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/copypipe.hpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/media/manager.hpp"
    
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/flight_data/client.hpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/flight_data/server.hpp"
)

# Packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavformat
    libavcodec
    libswscale>=6.1.100
    libavutil
)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
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
    PkgConfig::LIBAV
    Boost::system
    Threads::Threads
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
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
