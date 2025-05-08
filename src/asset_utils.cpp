struct MeshAsset
{
    std::vector<glm::vec4> vertices;
    std::vector<u32> indices;
};

template <>
struct fastgltf::ElementTraits<glm::vec3> : fastgltf::ElementTraitsBase<glm::vec3, AccessorType::Vec3, f32> {};

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

        fastgltf::Accessor& vertAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
        asset.vertices.reserve(asset.vertices.size() + vertAccessor.count);
        fastgltf::iterateAccessor<glm::vec3>(gltf, vertAccessor, [&](glm::vec3 pos)
        {
            asset.vertices.push_back(glm::vec4(-pos.z, pos.x, pos.y, 1.0f));
        });
    }

    return asset;
}