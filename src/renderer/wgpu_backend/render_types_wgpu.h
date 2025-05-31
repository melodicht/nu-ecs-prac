#pragma once

#include "math/math_consts.h"

#include <webgpu/webgpu.h>

// Represents the gpu side buffers representing a mesh within the WebGPU renderer
struct Mesh {
    // All of theses buffers and indices should be mapped already
    WGPUBuffer m_vertexBuffer;
    WGPUBuffer m_indexBuffer;
    u32 m_indexCount;

    Mesh(WGPUBuffer vertBuffer, WGPUBuffer indexBuffer, u32 indexCount) : 
        m_vertexBuffer(vertBuffer),
        m_indexBuffer(indexBuffer),
        m_indexCount(indexCount)
    {}
};