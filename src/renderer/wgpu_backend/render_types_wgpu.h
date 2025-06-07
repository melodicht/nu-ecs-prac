#pragma once

#include "math/math_consts.h"

#include <webgpu/webgpu.h>

// Represents location of a specified mesh within the WebGPU renderer
struct Mesh {
    u32 m_baseIndex{ 0 };
    u32 m_baseVertex{ 0 };
    u32 m_indexCount{ 0 };
    u32 m_vertexCount{ 0 };

    Mesh() : 
        m_baseIndex(),
        m_baseVertex(),
        m_indexCount(),
        m_vertexCount()
    {}

    Mesh(u32 baseIndex, u32 baseVertex, u32 indexCount, u32 vertexCount) : 
        m_baseIndex(baseIndex),
        m_baseVertex(baseVertex),
        m_indexCount(indexCount),
        m_vertexCount(vertexCount)
    {}
};