#include <toml++/toml.hpp>
#include <iostream>

#include "component_def.h"

struct FieldInfo
{
    const char *name;
    size_t size;
    void (*loadFunc)(char* dest, toml::node*);
};

struct ComponentInfo
{
    void (*loadFunc)(Scene&, EntityID, toml::table*, int);
    std::vector<FieldInfo> fields;
};

std::vector<ComponentInfo> compInfos;

template <typename T>
void LoadValue(char* dest, toml::node* data) = delete;

template <>
void LoadValue<int>(char* dest, toml::node* data)
{
    if (!data->is_integer())
    {
        std::cout << "This field must be an integer\n";
    }

    *(int*)dest = data->as_integer()->get();
}

template <>
void LoadValue<float>(char* dest, toml::node* data)
{
    if (!data->is_floating_point())
    {
        std::cout << "This field must be a floating point number\n";
    }

    *(float*)dest = data->as_floating_point()->get();
}

template <>
void LoadValue<bool>(char* dest, toml::node* data)
{
    if (!data->is_boolean())
    {
        std::cout << "This field must be a boolean\n";
    }

    *(bool*)dest = data->as_boolean()->get();
}

template <>
void LoadValue<glm::vec3>(char* dest, toml::node* data)
{
    if (!data->is_array())
    {
        std::cout << "This field must be an array\n";
    }

    toml::array* array = data->as_array();

    if (array->size() != 3)
    {
        std::cout << "This field must have a length of 3\n";
    }

    LoadValue<float>(dest, array->get(0));
    LoadValue<float>(dest + sizeof(float), array->get(1));
    LoadValue<float>(dest + (2 * sizeof(float)), array->get(2));
}

template <typename T>
void LoadComponent(Scene &scene, EntityID entity, toml::table* compData, int compIndex)
{
    char* comp = (char*)scene.Assign<T>(entity);
    ComponentInfo& compInfo = compInfos[compIndex];
    for (FieldInfo& field : compInfo.fields)
    {
        if (compData->contains(field.name))
        {
            field.loadFunc(comp, compData->get(field.name));
        }

        comp += field.size;
    }
}

template <>
void LoadComponent<MeshComponent>(Scene &scene, EntityID entity, toml::table* compData, int compIndex)
{
    MeshComponent* comp = scene.Assign<MeshComponent>(entity);

    if (!compData->contains("mesh"))
    {
        std::cout << "Must specify a mesh\n";
    }

    toml::node* meshData = compData->get("mesh");

    if (!meshData->is_string())
    {
        std::cout << "This field must be a string\n";
    }

    std::string meshPath = meshData->as_string()->get();

    if (!meshIDs.contains(meshPath))
    {
        std::cout << "Invalid mesh name\n";
    }

    comp->mesh = meshIDs[meshPath];

    if (compData->contains("color"))
    {
        LoadValue<glm::vec3>(((char*)comp) + sizeof(MeshID), compData->get("color"));
    }
}

template <typename T>
void AddComponent(Scene &scene, const char *name)
{
    compName<T> = name;
    MakeComponentId(name);
    scene.AddComponentPool(sizeof(T));
    compInfos.push_back({LoadComponent<T>});
}

template <typename T>
void AddField(const char *name)
{
    ComponentInfo& compInfo = compInfos[numComponents - 1];
    compInfo.fields.push_back({name, sizeof(T), LoadValue<T>});
}

template <typename T>
void AddLocalField(const char *name)
{
    ComponentInfo& compInfo = compInfos[numComponents - 1];
    compInfo.fields.push_back({name, sizeof(T)});
}

#define COMP(name) AddComponent<name>(scene, #name);
#define FIELD(type, name, start) AddField<type>(#name)
#define LOCAL_FIELD(type, name, start) AddLocalField<type>(#name)
#define LOCAL_DEF(def)

void RegisterComponents(Scene &scene)
{
    AddComponent<Transform3D>(scene, "Transform3D");
    AddField<glm::vec3>("position");
    AddField<glm::vec3>("rotation");
    AddField<glm::vec3>("scale");

    #include "components.h"
}

#undef COMP
#undef FIELD
#undef LOCAL_FIELD
#undef LOCAL_DEF

void LoadScene(Scene& scene, const char* filename)
{
    toml::table tbl;
    try
    {
        tbl = toml::parse_file(filename);
        toml::array* entities = tbl["entity"].as_array();

        for (toml::node& entity : *entities)
        {
            toml::table* table = entity.as_table();
            EntityID id = scene.NewEntity();

            for (auto val : *table)
            {
                if (!stringToId.contains(val.first.data()))
                {
                    std::cout << "Invalid Component: " << val.first.data() <<"\n";
                    exit(0);
                }

                int compIndex = stringToId[val.first.data()];
                ComponentInfo& compInfo = compInfos[compIndex];
                compInfo.loadFunc(scene, id, val.second.as_table(), compIndex);
            }
        }

    }
    catch (const toml::parse_error& error)
    {
        std::cout << error << '\n';
    }
}
