#include "Scene.hpp"

EntityID Scene::NewEntity()
{
    // std::vector::size runs in constant time.
    if (!freeIndices.empty())
    {
        unsigned int newIndex = freeIndices.back();
        freeIndices.pop_back();
        // Takes in index and incremented EntityVersion at that index
        EntityID newID = ECSconsts::CreateEntityId(newIndex, ECSconsts::GetEntityVersion(entities[newIndex].id));
        entities[newIndex].id = newID;
        return entities[newIndex].id;
    }
    entities.push_back({ ECSconsts::CreateEntityId((unsigned int)(entities.size()), 0), ComponentMask() });
    return entities.back().id;
}

// Removes a given entity from the scene and signals to the scene the free space that was left behind
void Scene::DestroyEntity(EntityID id)
{
    // Increments EntityVersion at the deleted index
    EntityID newID = ECSconsts::CreateEntityId((unsigned int)(-1), ECSconsts::GetEntityVersion(id) + 1);
    entities[ECSconsts::GetEntityIndex(id)].id = newID;
    entities[ECSconsts::GetEntityIndex(id)].mask.reset(); 
    freeIndices.push_back(ECSconsts::GetEntityIndex(id));
}