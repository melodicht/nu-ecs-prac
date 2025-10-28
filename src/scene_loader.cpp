#include <functional>

#include "component_def.h"

struct FieldInfo
{
    const char *name;
    size_t size;
    void (*loadFunc)(char* dest);
};

struct ComponentInfo
{
    void (*loadFunc)(Scene&, EntityID);
    std::vector<FieldInfo> fields;
};

std::vector<ComponentInfo> compInfos;

template <typename T>
void LoadValue(char* dest)
{

}

template <>
void LoadValue<int>(char* dest)
{

}

template <>
void LoadValue<float>(char* dest)
{

}

template <>
void LoadValue<bool>(char* dest)
{

}

template <>
void LoadValue<glm::vec3>(char* dest)
{

}

template <typename T>
void LoadComponent(Scene &scene, EntityID entity)
{
    T *comp = scene.Assign<T>(entity);
    LoadValue<T>((char *)comp);
}

template <typename T>
void AddComponent(Scene &scene, const char *name)
{
    compName<T> = name;
    u32 id = MakeComponentId(name);
    scene.componentPools[id] = new ComponentPool(sizeof(T));
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
    compInfo.fields.push_back({name});
}

#define COMP(name) AddComponent<name>(scene, "name");
#define FIELD(type, name, start) AddField<type>("name")
#define LOCAL_FIELD(type, name, start) AddLocalField<type>("name")
#define LOCAL_DEF(def)

void RegisterComponents(Scene &scene) {
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
