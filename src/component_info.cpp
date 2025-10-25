#include "component_def.h"

#define START_COMP_DEFS void makeComponentData() {
#define COMP(name) addComponent<name>("name");
#define FIELD(type, name, start) addField<type>("name", start)
#define END_COMP_DEFS }

#include "components.h"

#undef START_COMP_DEFS
#undef COMP
#undef FIELD
#undef END_COMP_DEFS
