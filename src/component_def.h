#ifndef COMPONENT_DEF_H
#define COMPONENT_DEF_H

#define START_COMP_DEFS
#define COMP(name) struct name
#define FIELD(type, name, start) type name = start
#define END_COMP_DEFS

#include "components.h"

#undef START_COMP_DEFS
#undef COMP
#undef FIELD
#undef END_COMP_DEFS

#endif //COMPONENT_DEF_H