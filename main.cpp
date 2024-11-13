#include <iostream>
#include <vector>
#include <bitset>

// for the ball game we make with the ECS
#define BALL_RADIUS 10

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

/*
 * SCENE DEFINITION + FUNCTIONALITY
 */

struct Scene {
  struct EntityEntry {
    EntityID id;  // though redundent with index in vector, required
                  // for deleting entities,
    ComponentMask mask;
  };
  std::vector<EntityEntry> entities;

  // Adds a new entity to this vector of entities, and returns its
  // ID. Can only support 2^64 entities without ID conflicts.
  EntityID NewEntity()
  {
    // std::vector::size runs in constant time.
    entities.push_back({ entities.size(), ComponentMask() });
    return entities.back().id;
  }

  // Assigns the entity associated with the given entity ID in this
  // vector of entities the given component.
  template<typename T>
  void Assign(EntityID id)
  {
    int componentId = GetId<T>();
    entities[id].mask.set(componentId);
  }
};

int main() {
  Scene scene;

  EntityID newEnt = scene.NewEntity();
  scene.Assign<TransformComponent>(newEnt);
}
