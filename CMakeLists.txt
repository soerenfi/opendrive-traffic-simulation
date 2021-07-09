cmake_minimum_required(VERSION 3.11.3)

project(xodr_traffic_sim)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty/tinyxml2)

set(SFML_DIR thirdparty/sfml/lib/cmake/SFML)
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

# Add project executable
add_executable(${PROJECT_NAME}
 src/main.cpp
src/tsim_map.cpp
src/tsim_object.cpp
src/opendrive_parser.cpp
src/tsim_simulator.cpp
src/tsim_map_builder.cpp
src/renderer_sfml.cpp
src/tsim_util.cpp
)

target_link_libraries(${PROJECT_NAME}
    sfml-graphics
    pthread 
    tinyxml2
)