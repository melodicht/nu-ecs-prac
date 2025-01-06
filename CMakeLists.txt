cmake_minimum_required(VERSION 3.13.4)

project(NUEngine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define sources (main.cpp and all .c and .cpp files in the src/ folder)
file(GLOB_RECURSE CPP_SOURCES ./src/*.cpp)
file(GLOB_RECURSE C_SOURCES ./src/*.c)

# actually create executable
add_executable(${PROJECT_NAME} code/sfm_ecsprac.cpp ${CPP_SOURCES} ${C_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR} code
)

# Link libraries
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-audio)