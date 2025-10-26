#include <string>

#include "component_def.h"

template <typename T>
void AddComponent(const char *name)
{
    componentName<T> = name;
}

template <typename T>
void AddField(const char *name, T start)
{

}

#define START_COMP_DEFS void MakeComponentData() {
#define COMP(name) AddComponent<name>("name");
#define FIELD(type, name, start) AddField<type>("name", start)
#define END_COMP_DEFS }

#include "components.h"

#undef START_COMP_DEFS
#undef COMP
#undef FIELD
#undef END_COMP_DEFS
