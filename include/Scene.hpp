#ifndef SCENE_HPP
#define SCENE_HPP

#include "ECSconsts.hpp"
#include "ComponentPool.hpp"

/*
 * SCENE DEFINITION + FUNCTIONALITY
 */

// Each component has its own memory pool, to have good memory
// locality. An entity's ID is the index into its own component in the
// component pool.
struct Scene {
  struct EntityEntry {
    EntityID id;  // though redundent with index in vector, required
                  // for deleting entities,
    ComponentMask mask;
  };
  std::vector<EntityEntry> entities;
  std::vector<ComponentPool*> componentPools;
  std::vector<unsigned int> freeIndices;

  // Adds a new entity to this vector of entities, and returns its
  // ID. Can only support 2^64 entities without ID conflicts.
  EntityID NewEntity();
  
  // Removes a given entity from the scene and signals to the scene the free space that was left behind
  void DestroyEntity(EntityID id);

  // Removes a component from the entity with the given EntityID
  // if the EntityID is not already removed.
  template<typename T>
  void Remove(EntityID id);

  // Assigns the entity associated with the given entity ID in this
  // vector of entities a new instance of the given component. Then,
  // adds it to its corresponding memory pool, and returns a pointer
  // to it.
  template<typename T>
  T* Assign(EntityID id);

  // Returns the pointer to the component instance on the entity
  // associated with the given ID in this vector of entities, with the
  // given component type. Returns nullptr if that entity doesn't have
  // the given component type.
  template<typename T>
  T* Get(EntityID id);
};

#endif