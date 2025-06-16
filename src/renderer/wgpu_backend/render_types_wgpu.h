#pragma once

#include "meta_definitions.h"

#include <webgpu/webgpu.h>

// Represents the gpu side buffers representing a mesh within the WebGPU renderer
struct Mesh {
    // All of theses buffers and indices should be mapped already
    WGPUBuffer m_vertexBuffer{ };
    WGPUBuffer m_indexBuffer{ };
    u32 m_indexCount{ 0 };
    u32 m_vertexCount{ 0 };

    Mesh() : 
        m_vertexBuffer(),
        m_indexBuffer(),
        m_indexCount(),
        m_vertexCount()
    {}

    Mesh(WGPUBuffer vertBuffer, WGPUBuffer indexBuffer, u32 indexCount, u32 vertexCount) : 
        m_vertexBuffer(vertBuffer),
        m_indexBuffer(indexBuffer),
        m_indexCount(indexCount),
        m_vertexCount(vertexCount)
    {}
};
