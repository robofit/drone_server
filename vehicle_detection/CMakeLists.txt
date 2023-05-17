cmake_minimum_required (VERSION 3.11)

project (vehicle_detection)

set(SOURCES
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/controller.cpp"
    "src/${SOLUTION_NAME}/${PROJECT_NAME}/vehicle_detection.cpp"
)

set(INCLUDES

)

# Packages
find_package(OpenCV REQUIRED)
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
    DDS::websocket
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
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
