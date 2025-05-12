cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("untitled-engine")

include(cmake/CPM.cmake)
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.12")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:spnda/fastgltf#v0.8.0")
CPMAddPackage("gh:ocornut/imgui#v1.91.9b")
find_package(Vulkan REQUIRED)


add_definitions(-DIMGUI_IMPL_VULKAN_USE_VOLK)

file(GLOB imgui_sources ${imgui_SOURCE_DIR}/*.cpp)
list(APPEND imgui_sources ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

add_executable(untitled-engine ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/VkBootstrap.cpp ${imgui_sources})
target_include_directories(untitled-engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(untitled-engine
        PUBLIC
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
)

target_link_libraries(untitled-engine PRIVATE Vulkan::Vulkan)
target_link_libraries(untitled-engine PRIVATE SDL3::SDL3)
target_link_libraries(untitled-engine PRIVATE glm::glm)
target_link_libraries(untitled-engine PRIVATE fastgltf::fastgltf)

if (WIN32)
    add_custom_command(TARGET untitled-engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:untitled-engine> $<TARGET_FILE_DIR:untitled-engine>
            COMMAND_EXPAND_LISTS
    )
endif()

set(SHADERS_IN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(GLOB_RECURSE SHADERS "${SHADERS_IN_DIR}/*.slang")

set(SLANGC "slangc")

file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SHADER_OUT_NAME "${SHADERS_OUT_DIR}/${SHADER_NAME}.spv")
    list(APPEND SHADER_OUT_NAMES ${SHADER_OUT_NAME})
    add_custom_command(
            MAIN_DEPENDENCY ${SHADER}
            OUTPUT ${SHADER_OUT_NAME}
            COMMAND ${SLANGC} ${SHADER} -o ${SHADER_OUT_NAME} -target spirv -fvk-use-entrypoint-name -emit-spirv-directly -fvk-use-gl-layout -matrix-layout-column-major
            VERBATIM)
endforeach()

add_custom_command(TARGET untitled-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_CURRENT_SOURCE_DIR}/data" ${CMAKE_BINARY_DIR}
)

add_custom_target(build_shaders DEPENDS ${SHADER_OUT_NAMES})

add_dependencies(untitled-engine build_shaders)
