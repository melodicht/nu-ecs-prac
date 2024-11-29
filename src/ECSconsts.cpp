#include "ECSconsts.hpp"

// EntityID ECSconsts::CreateEntityId(unsigned int index, unsigned int version)
// {
//     // Shift the index up 32, and put the version in the bottom
//     return ((EntityID)index << 32) | ((EntityID)version);
// }

// // This should represent the index an element has within the "entities" vector inside of a Scene
// unsigned int ECSconsts::GetEntityIndex(EntityID id)
// {
//     // Shift down 32 so we lose the version and get our index
//     return id >> 32;
// }
// unsigned int ECSconsts::GetEntityVersion(EntityID id)
// {
//     // Cast to a 32 bit int to get our version number (loosing the top 32 bits)
//     return (unsigned int)id;
// }

// // Checks if the EntityID has not been deleted
// bool ECSconsts::IsEntityValid(EntityID id)
// {
//     // Check if the index is our invalid index
//     return (id >> 32) != (unsigned int)(-1);
// }

// template <class T>
// int ECSconsts::GetId()
// {
//   static u64 s_componentId = s_componentCounter++;
//   return s_componentId;
// }