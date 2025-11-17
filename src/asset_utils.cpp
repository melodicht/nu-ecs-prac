#include "asset_types.h"

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

template <>
struct fastgltf::ElementTraits<glm::vec3> : fastgltf::ElementTraitsBase<glm::vec3, AccessorType::Vec3, f32> {};

template <>
struct fastgltf::ElementTraits<glm::vec2> : fastgltf::ElementTraitsBase<glm::vec3, AccessorType::Vec2, f32> {};

std::unordered_map<std::string, MeshID> meshIDs;
std::unordered_map<std::string, TextureID> texIDs;

PLATFORM_LOAD_MESH_ASSET(LoadMeshAsset)
{
    if (meshIDs.contains(name))
    {
        return meshIDs[name];
    }

    std::filesystem::path path = "models/" + name + ".glb";
    fastgltf::Expected<fastgltf::GltfDataBuffer> dataFile = fastgltf::GltfDataBuffer::FromPath(path);
    fastgltf::GltfDataBuffer data;
    if (dataFile)
    {
        data = std::move(dataFile.get());
    }
    else
    {
        return -1;
    }

    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;

    fastgltf::Asset gltf;
    fastgltf::Parser parser{};

    fastgltf::Expected<fastgltf::Asset> load = parser.loadGltfBinary(data, path.parent_path(), gltfOptions);
    if (load)
    {
        gltf = std::move(load.get());
    }
    else
    {
        return -1;
    }

    fastgltf::Mesh mesh = gltf.meshes[0];
    MeshAsset asset;

    for (fastgltf::Primitive &p : mesh.primitives)
    {
        fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
        asset.indices.reserve(asset.indices.size() + indexAccessor.count);
        fastgltf::iterateAccessor<u32>(gltf, indexAccessor, [&](u32 index)
        {
            asset.indices.push_back(index);
        });

        fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
        fastgltf::Accessor& normAccessor = gltf.accessors[p.findAttribute("NORMAL")->accessorIndex];
        fastgltf::Accessor& uvAccessor = gltf.accessors[p.findAttribute("TEXCOORD_0")->accessorIndex];
        asset.vertices.reserve(asset.vertices.size() + posAccessor.count);
        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](glm::vec3 pos, u32 index)
        {
            glm::vec3 norm = fastgltf::getAccessorElement<glm::vec3>(gltf, normAccessor, index);
            glm::vec2 uv = fastgltf::getAccessorElement<glm::vec2>(gltf, uvAccessor, index);

            Vertex vert;
            vert.position = {-pos.z, pos.x, pos.y};
            vert.normal = {-norm.z, norm.x, norm.y};
            vert.uvX = uv.x;
            vert.uvY = uv.y;

            asset.vertices.push_back(vert);
        });
    }

    RenderUploadMeshInfo info{};
    info.vertData = asset.vertices.data();
    info.vertSize = asset.vertices.size();
    info.idxData = asset.indices.data();
    info.idxSize = asset.indices.size();

    MeshID id = UploadMesh(info);
    meshIDs[name] = id;

    return id;
}

PLATFORM_LOAD_TEXTURE_ASSET(LoadTextureAsset)
{
    if (texIDs.contains(name))
    {
        return meshIDs[name];
    }

    std::filesystem::path path = "textures/" + name + ".png";

    int width, height, channels;

    stbi_uc* imageData = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    TextureAsset asset{};
    asset.width = width;
    asset.height = height;
    asset.pixels = std::vector<u32>((width * height * 4) / sizeof(u32));

    memcpy(asset.pixels.data(), imageData, asset.pixels.size() * sizeof(u32));
    stbi_image_free(imageData);

    RenderUploadTextureInfo info{};
    info.width = asset.width;
    info.height = asset.height;
    info.pixelData = asset.pixels.data();

    TextureID id = UploadTexture(info);
    texIDs[name] = id;

    return id;
}
