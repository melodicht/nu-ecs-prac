#pragma once

#include "math/skl_math_consts.h"
#include "renderer/render_types.h"

#include <vector>

struct MeshAsset
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

struct TextureAsset
{
    u32 width;
    u32 height;

    std::vector<u32> pixels;
};