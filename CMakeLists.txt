cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("untitled-engine")

# Resolves dependencies
include(cmake/CPM.cmake)

add_library(DEPENDENCIES INTERFACE)
set(INCLUDED ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer)
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Determines rendering backend
set(render_sys "VULKAN")
if(APPLE)
        set(render_sys "WEBGPU")
endif()

message("Initializing [${render_sys}] rendering backend")

# Accordingly resolves dependencies required for each rendering backend
if(${render_sys} STREQUAL "VULKAN")
        find_package(Vulkan REQUIRED)
        target_link_libraries(DEPENDENCIES INTERFACE Vulkan::Vulkan)
        list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vk_backend/VkBootstrap.cpp)
        list(APPEND INCLUDED ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan)
elseif(${render_sys} STREQUAL "WEBGPU")
        # TODO: Make Webgpu back end
        list(APPEND INCLUDED ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/wpu_backend)
endif()

# Resolves dependencies that always are needed
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.12")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:spnda/fastgltf#v0.8.0")
target_link_libraries(DEPENDENCIES INTERFACE SDL3::SDL3)
target_link_libraries(DEPENDENCIES INTERFACE glm::glm)
target_link_libraries(DEPENDENCIES INTERFACE fastgltf::fastgltf)


# Puts together target
add_executable(untitled-engine ${SRC})
target_include_directories(untitled-engine PUBLIC ${INCLUDED})
target_link_libraries(untitled-engine PRIVATE DEPENDENCIES)

# TODO: IDK what this is xd
if (WIN32)
    add_custom_command(TARGET untitled-engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:untitled-engine> $<TARGET_FILE_DIR:untitled-engine>
            COMMAND_EXPAND_LISTS
    )
endif()

# Includes shaders in build
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
