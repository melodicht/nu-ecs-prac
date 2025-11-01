#include "asset_types.h"

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

template <>
struct fastgltf::ElementTraits<glm::vec3> : fastgltf::ElementTraitsBase<glm::vec3, AccessorType::Vec3, f32> {};

template <>
struct fastgltf::ElementTraits<glm::vec2> : fastgltf::ElementTraitsBase<glm::vec3, AccessorType::Vec2, f32> {};

std::unordered_map<std::string, MeshID> meshIDs;

MeshAsset LoadMeshAsset(std::filesystem::path path)
{
    fastgltf::Expected<fastgltf::GltfDataBuffer> dataFile = fastgltf::GltfDataBuffer::FromPath(path);
    fastgltf::GltfDataBuffer data;
    if (dataFile)
    {
        data = std::move(dataFile.get());
    }
    else
    {
        return {};
    }

    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;

    fastgltf::Asset gltf;
    fastgltf::Parser parser{};

    fastgltf::Expected<fastgltf::Asset> load = parser.loadGltfBinary(data, path.parent_path());
    if (load)
    {
        gltf = std::move(load.get());
    }
    else
    {
        return {};
    }

    fastgltf::Mesh mesh = gltf.meshes[0];
    MeshAsset asset;

    for (fastgltf::Primitive &p : mesh.primitives)
    {
        u32 startIndex = asset.vertices.size();
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

    return asset;
}

void LoadMeshes()
{
    for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator("models"))
    {
        MeshAsset asset = LoadMeshAsset(file.path());

        RenderUploadMeshInfo uploadInfo
        {
                .vertData = asset.vertices.data(),
                .idxData = asset.indices.data(),
                .vertSize = (u32)asset.vertices.size(),
                .idxSize = (u32)asset.indices.size()
        };

        meshIDs[file.path().stem().string()] = UploadMesh(uploadInfo);
    }
}

TextureAsset LoadTextureAsset(const char *path)
{
    int width, height, channels;

    stbi_uc* imageData = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    TextureAsset asset{};
    asset.width = width;
    asset.height = height;
    asset.pixels = std::vector<u32>((width * height * channels) / sizeof(u32));

    memcpy(asset.pixels.data(), imageData, asset.pixels.size() * sizeof(u32));
    stbi_image_free(imageData);

    return asset;
}
