#include <functional>

#include "component_def.h"

struct FieldInfo
{
    const char *fieldName;

};

struct ComponentInfo
{
    std::function<void(Scene&, EntityID)> loadFunc;
    std::vector<FieldInfo> fields;
};

std::vector<ComponentInfo> compInfos;

template <typename T>
void LoadValue(char* dest) = delete;

#define START_COMP_DEFS
#define COMP(name) template <> void LoadValue<name>(char* dest)
#define FIELD(type, name, start)
#define END_COMP_DEFS

#include "components.h"

#undef START_COMP_DEFS
#undef COMP
#undef FIELD
#undef END_COMP_DEFS

template <typename T>
void LoadComponent(Scene &scene, EntityID entity)
{
    T *comp = scene.Assign<T>(entity);
    LoadValue<T>((char *)comp);
}

template <typename T>
void AddComponent(Scene &scene, const char *name)
{
    componentName<T> = name;
    u32 id = MakeComponentId(name);
    scene.componentPools[id] = new ComponentPool(sizeof(T));
    compInfos.push_back({LoadComponent<T>});
}

#define START_COMP_DEFS void RegisterComponents(Scene &scene) {
#define COMP(name) AddComponent<name>(scene, "name");
#define FIELD(type, name, start)
#define END_COMP_DEFS }

#include "components.h"

#undef START_COMP_DEFS
#undef COMP
#undef FIELD
#undef END_COMP_DEFS
