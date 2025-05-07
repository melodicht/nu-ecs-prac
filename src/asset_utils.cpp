struct MeshAsset
{
    std::vector<glm::vec3> vertices;
    std::vector<u32> indices;
};

MeshAsset LoadMeshAsset(std::filesystem::path path)
{
    fastgltf::GltfDataBuffer data;
    data.FromPath(path);

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

    }


    return asset;
}