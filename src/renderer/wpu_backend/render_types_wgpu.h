#pragma once

#include "math/math_consts.h"

#include <webgpu/webgpu.h>

struct Mesh {
    // All of the following buffers should be initialized when
    WGPUBuffer m_vertexBuffer;
    WGPUBuffer m_indexBuffer;
    u32 m_indexCount;
};