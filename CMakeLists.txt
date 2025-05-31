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

# Takes in an option for rendering backend
set(render_sys "Default" CACHE STRING "Which graphics API should the rendering backend be based on")
set_property(CACHE render_sys PROPERTY STRINGS "Default" "WebGPU" "Vulkan")

# Takes in other options
option(enable_logging "Whether logging should be enabled or not" ON)
if(${enable_logging})
        target_compile_definitions(DEPENDENCIES INTERFACE SKL_LOGGING_ENABLED=1)
else()
        target_compile_definitions(DEPENDENCIES INTERFACE SKL_LOGGING_ENABLED=0)
endif()

# Determines rendering backend if none is explicitly picked
if(${render_sys} STREQUAL "Default")
        message("Assigning rendering backend based on machine...")
        if(APPLE)
                # Currently MoltenVK has not caught up in terms of SDK to the currently used vulkan version
                # Maybe we should change once that becomes avaliable
                set(render_sys "WebGPU")
        elseif(EMSCRIPTEN)
                # No other options for web
                set(render_sys "WebGPU")
        else()
                # Likely Vulkan will run better on the things it is natively supported by
                set(render_sys "Vulkan")
        endif()

# Checks if selected option is available for current device, throws error if not
else()
        message("Checking for api support on current machine...")
        # Web and Apple simply do not work with anything outside of WebGPU
        if(EMSCRIPTEN)
                if(NOT ${render_sys} STREQUAL "WebGPU")
                        message(FATAL_ERROR "Web builds need to use WebGPU backend")
                endif()
        elseif(APPLE)
                if(NOT ${render_sys} STREQUAL "WebGPU")
                        message(FATAL_ERROR "Apple devices currently only support WebGPU backends")
                endif()
        endif()
endif()

# Accordingly resolves dependencies required for each rendering backend
message("Initializing [${render_sys}] rendering backend...")
if(${render_sys} STREQUAL "Vulkan")
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
        
        target_compile_definitions(DEPENDENCIES INTERFACE
                SKL_RENDERER=0
        )

        target_link_libraries(DEPENDENCIES INTERFACE vulkan-backend)

elseif(${render_sys} STREQUAL "WebGPU")

        # Includes backend dll as part of projects
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/wgpu_backend wgpu-backend)
        target_compile_definitions(wgpu-backend PRIVATE SKL_LOGGING_ENABLED=1)
        target_link_libraries(DEPENDENCIES INTERFACE
                wgpu-backend
        )

        # Allows for switching off certain pieces of logic not supported by certain backends
        target_compile_definitions(DEPENDENCIES INTERFACE 
                SKL_RENDERER=1
        )
endif()

# Puts together target
add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>)
target_link_libraries(${PROJECT_NAME} PRIVATE DEPENDENCIES)
if (EMSCRIPTEN)
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )
endif()

# Deals with moving files into build folder

# Deals with local compilation
if(NOT EMSCRIPTEN)
        # Includes shaders in build
        if(render_sys STREQUAL "Vulkan")
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
                        COMMAND ${SLANGC} ${SHADER} -o ${SHADER_OUT_NAME} -target spirv -fvk-use-entrypoint-name -emit-spirv-directly -fvk-use-scalar-layout -matrix-layout-column-major
                        VERBATIM)
                endforeach()

                add_custom_target(build_shaders DEPENDS ${SHADER_OUT_NAMES})

                add_dependencies(${PROJECT_NAME} build_shaders)
        else(render_sys STREQUAL "WebGPU")
                add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
                        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders/webgpu" "${CMAKE_BINARY_DIR}/shaders"
                )
        endif()

        # Imports other resources into build
        add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_CURRENT_SOURCE_DIR}/data" ${CMAKE_BINARY_DIR}
        )

# Preloads files for emscripten
else()
        target_link_options(${PROJECT_NAME} PRIVATE
                "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/shaders/webgpu@/shaders/" #This isn't great, would be better to encapsulate all of this into a specific folder
                "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/data/models@/models/"
                "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/data/fonts@/fonts/"
                --preload-plugins
        )
endif()