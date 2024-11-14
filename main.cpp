#include <iostream>
#include <vector>
#include <bitset>

// for the ball game we make with the ECS
#define BALL_RADIUS 10

#define u8  uint8_t
#define u32 uint32_t
#define u64 uint64_t

//////////////// COMPONENTS ////////////////

// Gets a unique identifier for the given component T, which is
// guaranteed to be different from previous outputs for only the first
// 2^64 unsigned ints. 
u64 s_componentCounter;
template <class T>
int GetId()
{
  static u64 s_componentId = s_componentCounter++;
  return s_componentId;
}
/*
 * Examples
 * GetId<TransformComponent>() -> 0
 * GetId<TransformComponent>() -> 0
 * GetId<BallPhysics>() -> 1
 * GetId<TransformComponent>() -> 0
 * GetId<BallPhysics>() -> 1
 */

// Represents a point in a 2D space as a Component
class TransformComponent {
    float x_pos;
    float y_pos;
};



/*
 * TYPE DEFINITIONS AND CONSTANTS
 */

typedef u64 EntityID;
const u32 MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
const u32 MAX_ENTITIES = 256;

/*
 * COMPONENT POOL
 */

// Responsible for allocating contiguous memory for the components
// such that `MAX_ENTITIES` can be stored, and components be accessed
// via index.
// NOTE: The memory pool is an array of bytes, as the size of one
// component isn't known at compile time.
struct ComponentPool
{
  ComponentPool(size_t elementsize)
  {
    // We'll allocate enough memory to hold MAX_ENTITIES, each with element size
    elementSize = elementsize;
    pData = new u8[elementSize * MAX_ENTITIES];
  }

  ~ComponentPool()
  {
    delete[] pData;
  }

  // Gets the component in this pData at the given index.
  inline void* get(size_t index)
  {
    // looking up the component at the desired index
    return pData + index * elementSize;
  }

  u8* pData{ nullptr };
  size_t elementSize{ 0 };
};

/*
 * SCENE DEFINITION + FUNCTIONALITY
 */

// Each component has its own memory pool, to have good memory
// locality.
struct Scene {
  struct EntityEntry {
    EntityID id;  // though redundent with index in vector, required
                  // for deleting entities,
    ComponentMask mask;
  };
  std::vector<EntityEntry> entities;
  std::vector<ComponentPool*> componentPools;

  // Adds a new entity to this vector of entities, and returns its
  // ID. Can only support 2^64 entities without ID conflicts.
  EntityID NewEntity()
  {
    // std::vector::size runs in constant time.
    entities.push_back({ entities.size(), ComponentMask() });
    return entities.back().id;
  }

  // Assigns the entity associated with the given entity ID in this
  // vector of entities a new instance of the given component. Then,
  // adds it to its corresponding memory pool, and returns a pointer
  // to it.
  template<typename T>
  T* Assign(EntityID id)
  {
    int componentId = GetId<T>();

    if (componentPools.size() <= componentId) // Not enough component pool
    {
      componentPools.resize(componentId + 1, nullptr);
    }
    if (componentPools[componentId] == nullptr) // New component, make a new pool
    {
      componentPools[componentId] = new ComponentPool(sizeof(T));
    }

    // Looks up the component in the pool, and initializes it with placement new
    T* pComponent = new (componentPools[componentId]->get(id)) T();

    entities[id].mask.set(componentId);
    return pComponent;
  }

  // Returns the pointer to the component instance on the entity
  // associated with the given ID in this vector of entities, with the
  // given component type. Returns nullptr if that entity doesn't have
  // the given component type.
  template<typename T>
  T* Get(EntityID id)
  {
    int componentId = GetId<T>();
    if (!entities[id].mask.test(componentId))
      return nullptr;

    T* pComponent = static_cast<T*>(componentPools[componentId]->get(id));
    return pComponent;
  }
};

int main() {
  Scene scene;

  EntityID newEnt = scene.NewEntity();
  scene.Assign<TransformComponent>(newEnt);
}
