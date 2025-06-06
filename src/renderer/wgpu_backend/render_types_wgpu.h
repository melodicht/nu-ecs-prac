#pragma once

#include "math/math_consts.h"

#include <webgpu/webgpu.h>

// Represents the gpu side buffers representing a mesh within the WebGPU renderer
struct Mesh {
    // All of theses buffers and indices should be mapped already
    WGPUBuffer m_indexBuffer{ };
    u32 m_indexCount{ 0 };
    u32 m_vertexBaseIdx{ 0 };   // Starting point in mesh buffer
    u32 m_vertexCount{ 0 };

    Mesh() : 
        m_indexBuffer(),
        m_indexCount(),
        m_vertexBaseIdx(),
        m_vertexCount()
    {}

    Mesh(WGPUBuffer indexBuffer, u32 indexCount, u32 vertexBaseIdx, u32 vertexCount) : 
        m_indexBuffer(indexBuffer),
        m_indexCount(indexCount),
        m_vertexBaseIdx(vertexBaseIdx),
        m_vertexCount(vertexCount)
    {}
};