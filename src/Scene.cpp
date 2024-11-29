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

// Removes a component from the entity with the given EntityID
// if the EntityID is not already removed.
template<typename T>
void Scene::Remove(EntityID id)
{
    // ensures you're not accessing an entity that has been deleted
    if (entities[ECSconsts::GetEntityIndex(id)].id != id) 
        return;

    int componentId = ECSconsts::GetId<T>();
    // Finds location of component data within the entity component pool and 
    // resets, thus removing the component from the entity
    entities[ECSconsts::GetEntityIndex(id)].mask.reset(componentId);
}

// Assigns the entity associated with the given entity ID in this
// vector of entities a new instance of the given component. Then,
// adds it to its corresponding memory pool, and returns a pointer
// to it.
template<typename T>
T* Scene::Assign(EntityID id)
{
    int componentId = ECSconsts::GetId<T>();

    if (componentPools.size() <= componentId) // Not enough component pool
    {
        componentPools.resize(componentId + 1, nullptr);
    }
    if (componentPools[componentId] == nullptr) // New component, make a new pool
    {
        componentPools[componentId] = new ComponentPool(sizeof(T));
    }

    // Looks up the component in the pool, and initializes it with placement new
    T* pComponent = new (componentPools[componentId]->get(ECSconsts::GetEntityIndex(id))) T();

    entities[ECSconsts::GetEntityIndex(id)].mask.set(componentId);
    return pComponent;
}

// Returns the pointer to the component instance on the entity
// associated with the given ID in this vector of entities, with the
// given component type. Returns nullptr if that entity doesn't have
// the given component type.
template<typename T>
T* Scene::Get(EntityID id)
{
    int componentId = ECSconsts::GetId<T>();
    if (!entities[ECSconsts::GetEntityIndex(id)].mask.test(componentId))
        return nullptr;

    T* pComponent = static_cast<T*>(componentPools[componentId]->get(ECSconsts::GetEntityIndex(id)));
    return pComponent;
}