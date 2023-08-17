cmake_minimum_required (VERSION 3.8)

project ("DDS")

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(SOLUTION_NAME "${PROJECT_NAME}")
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")

SET(COMPILE_RTMP ON CACHE BOOL "should be RTMP module compiled?")
SET(COMPILE_WEBSOCKET ON CACHE BOOL "should be WebSocket module compiled?")

if(COMPILE_WEBSOCKET)
	SET(COMPILE_VEHICLE_DETECTION ON CACHE BOOL "should be Vehicle Detection module compiled?")
	SET(COMPILE_RECORDER ON CACHE BOOL "should be Recorder module compiled?")
	SET(COMPILE_FLIGHT_DATABASE ON CACHE BOOL "should be FlightDatabase module compiled?")
endif()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

SET(CMAKE_CXX_STANDARD 14)

#set(CMAKE_VERBOSE_MAKEFILE on)

add_compile_definitions(DLLDIR_EX)

#export
configure_file(config.hpp.in DDS/config.hpp)

install(DIRECTORY
	"${CMAKE_CURRENT_BINARY_DIR}/DDS"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


add_subdirectory ("core")

if(COMPILE_RTMP)
	add_subdirectory ("rtmp")
endif()

if(COMPILE_WEBSOCKET)
	add_subdirectory ("websocket")
endif()

if(COMPILE_RECORDER)
	add_subdirectory(recorder)
endif()

if(COMPILE_FLIGHT_DATABASE)
	add_subdirectory(flight_database)
endif()

if(COMPILE_VEHICLE_DETECTION)
	add_subdirectory(vehicle_detection)
endif()


find_package(Boost REQUIRED COMPONENTS system program_options)

add_executable (${PROJECT_NAME}a
    "DDS.cpp"
)

target_include_directories(${PROJECT_NAME}a PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

target_link_libraries(${PROJECT_NAME}a
	DDS::core
	Boost::program_options
)

if(COMPILE_RTMP)
	target_link_libraries(${PROJECT_NAME}a DDS::rtmp)
endif()

if(COMPILE_WEBSOCKET)
	target_link_libraries(${PROJECT_NAME}a DDS::websocket)
endif()

if(COMPILE_RECORDER)
	target_link_libraries(${PROJECT_NAME}a DDS::recorder)
endif()

if(COMPILE_FLIGHT_DATABASE)
	target_link_libraries(${PROJECT_NAME}a DDS::flight_database)
endif()

if(COMPILE_VEHICLE_DETECTION)
	target_link_libraries(${PROJECT_NAME}a DDS::vehicle_detection)
endif()

install(TARGETS ${PROJECT_NAME}a
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
