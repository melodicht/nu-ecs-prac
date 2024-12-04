#include "Scene.hpp"

EntityID Scene::NewEntity()
{
    // std::vector::size runs in constant time.
    if (!freeIndices.empty())
    {
        unsigned int newIndex = freeIndices.back();
        freeIndices.pop_back();
        // Takes in index and incremented EntityVersion at that index
        EntityID newID = ECSConsts::CreateEntityId(newIndex, ECSConsts::GetEntityVersion(entities[newIndex].id));
        entities[newIndex].id = newID;
        return entities[newIndex].id;
    }
    entities.push_back({ ECSConsts::CreateEntityId((unsigned int)(entities.size()), 0), ComponentMask() });
    return entities.back().id;
}

// Removes a given entity from the scene and signals to the scene the free space that was left behind
void Scene::DestroyEntity(EntityID id)
{
    // Increments EntityVersion at the deleted index
    EntityID newID = ECSConsts::CreateEntityId((unsigned int)(-1), ECSConsts::GetEntityVersion(id) + 1);
    entities[ECSConsts::GetEntityIndex(id)].id = newID;
    entities[ECSConsts::GetEntityIndex(id)].mask.reset(); 
    freeIndices.push_back(ECSConsts::GetEntityIndex(id));
}