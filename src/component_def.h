#ifndef COMPONENT_DEF_H
#define COMPONENT_DEF_H

#define COMP(name) struct name
#define FIELD(type, name, start) type name = start
#define LOCAL_FIELD(type, name, start) type name = start
#define LOCAL_DEF(def) def

#include "components.h"

#undef COMP
#undef FIELD
#undef LOCAL_FIELD
#undef LOCAL_DEF

#endif //COMPONENT_DEF_H