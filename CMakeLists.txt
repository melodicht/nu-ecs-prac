cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("untitled-engine")

# Resolves dependencies
include(cmake/CPM.cmake)

add_library(DEPENDENCIES INTERFACE)
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Resolves dependencies that always are needed
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.12")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:spnda/fastgltf#v0.8.0")
CPMAddPackage("gh:ocornut/imgui#v1.91.9b")
target_link_libraries(DEPENDENCIES INTERFACE SDL3::SDL3)
target_link_libraries(DEPENDENCIES INTERFACE glm::glm)
target_link_libraries(DEPENDENCIES INTERFACE fastgltf::fastgltf)


file(GLOB imgui_sources ${imgui_SOURCE_DIR}/*.cpp)
add_library(imgui SHARED ${imgui_sources})
target_include_directories(imgui PRIVATE $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>)
list(APPEND SRC ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp)

target_link_libraries(DEPENDENCIES INTERFACE imgui)

# Determines rendering backend
set(render_sys "VULKAN")
if(APPLE)
        set(render_sys "WEBGPU")
        enable_language(OBJC)
elseif(EMSCRIPTEN)
        set(render_sys "WEBGPU")
endif()

message("Initializing [${render_sys}] rendering backend")

# Accordingly resolves dependencies required for each rendering backend
if(${render_sys} STREQUAL "VULKAN")
        add_definitions(-DIMGUI_IMPL_VULKAN_USE_VOLK)
        find_package(Vulkan REQUIRED)
        set(VULKAN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vk_backend)
        add_library(vulkan-backend SHARED
                ${VULKAN_SRC_DIR}/renderer_vk.cpp
                ${VULKAN_SRC_DIR}/VkBootstrap.cpp
                ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)
        target_link_libraries(vulkan-backend PRIVATE Vulkan::Vulkan)
        target_link_libraries(vulkan-backend PRIVATE SDL3::SDL3)
        target_link_libraries(vulkan-backend PRIVATE glm::glm)
        target_link_libraries(vulkan-backend PRIVATE imgui)
        target_include_directories(vulkan-backend PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>)
        target_link_libraries(DEPENDENCIES INTERFACE vulkan-backend)
elseif(${render_sys} STREQUAL "WEBGPU")
        # Includes dawn backend
        list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/wpu_backend/renderer_wpu.cpp)

        set(WEBGPU_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/wpu_backend)
        add_library(wpu-backend ${WEBGPU_SRC_DIR}/renderer_wpu.cpp ${WEBGPU_SRC_DIR}/render_backend_wpuimpl.cpp)
        target_include_directories(wpu-backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)

        if(EMSCRIPTEN)
                target_link_options(wpu-backend PUBLIC 
                        -sUSE_WEBGPU 
                        -sUSE_SDL=3
                        -sASYNCIFY)
        else()
                # Fetches Dawn
                set(DAWN_FETCH_DEPENDENCIES ON)
                CPMAddPackage(NAME dawn URL "https://dawn.googlesource.com/dawn/+archive/refs/heads/main.tar.gz")
                target_link_libraries(wpu-backend PUBLIC dawn::webgpu_dawn)
        endif()

        target_link_libraries(wpu-backend PUBLIC SDL3::SDL3)
        target_link_libraries(wpu-backend PUBLIC glm::glm)
        target_link_libraries(DEPENDENCIES INTERFACE wpu-backend)
endif()

# Puts together target
add_executable(untitled-engine ${SRC})
target_include_directories(untitled-engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>)
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
