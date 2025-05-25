add_library(wpu-backend STATIC ${CMAKE_CURRENT_SOURCE_DIR}/renderer_wpu.cpp ${CMAKE_CURRENT_SOURCE_DIR}/render_backend_wpuimpl.cpp)

target_include_directories(wpu-backend PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# Needs this to suppress cmake warning
if(APPLE)
    enable_language(OBJC)
endif()


add_library(webgpu INTERFACE)
set(DAWN_FETCH_DEPENDENCIES ON)
if(EMSCRIPTEN)
        # Fetches emscripten Dawn port
        # Got a lot of help from "https://github.com/eliemichel/WebGPU-distribution/blob/main/emdawnwebgpu/CMakeLists.txt"
        CPMAddPackage(NAME emdawn URL "https://github.com/google/dawn/releases/download/v20250519.170457/emdawnwebgpu_pkg-v20250519.170457.zip")
        set(emdawn_ROOT "${emdawn_SOURCE_DIR}")
        message(${emdawn_ROOT})
        target_link_options(webgpu INTERFACE 
                -sASSERTIONS
                -sUSE_SDL=3
                -sASYNCIFY
                -O0
                -EMCC_DEBUG=1
                -sALLOW_MEMORY_GROWTH
                --use-port=${emdawn_ROOT}/emdawnwebgpu.port.py
                --closure-args=--externs=${emdawn_ROOT}/webgpu/src/webgpu-externs.js)
        target_compile_options(webgpu INTERFACE 
                --use-port=${emdawn_ROOT}/emdawnwebgpu.port.py)
else()
        # Fetches Dawn
        CPMAddPackage(NAME dawn URL "https://dawn.googlesource.com/dawn/+archive/refs/heads/main.tar.gz")
        target_link_libraries(webgpu INTERFACE dawn::webgpu_dawn)
endif()

# Includes ported SDL3-webgpu conduit
add_subdirectory(${CMAKE_SOURCE_DIR}/include/webgpu/sdl3webgpu-main sdl3webgpu)

target_link_libraries(wpu-backend PRIVATE glm::glm sdl3webgpu)