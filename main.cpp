#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>
#include <bitset>

// for the ball game we make with the ECS
#define BALL_RADIUS 20
#define NUM_BALLS 200

// For rendering

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

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
    public:
        float x_pos;
        float y_pos;
};

struct Rigidbody {
  float v_x;
  float v_y;
};

struct CircleCollider {
  float radius;
};



/*
 * TYPE DEFINITIONS AND CONSTANTS
 */

typedef u64 EntityID;
const u32 MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
const u32 MAX_ENTITIES = 256;

/*
 * ID FUNCTIONALITY 
 */

// Allows the storage of EntityID's such that they store both the Index and their Version number
// This allows for deletion without overlapping slots.
// Index and Version number are both 32 bit unsigned ints that will be combined into EntityID.

inline EntityID CreateEntityId(unsigned int index, unsigned int version)
{
  // Shift the index up 32, and put the version in the bottom
  return ((EntityID)index << 32) | ((EntityID)version);
}
// This should represent the index an element has within the "entities" vector inside of a Scene
inline unsigned int GetEntityIndex(EntityID id)
{
  // Shift down 32 so we lose the version and get our index
  return id >> 32;
}

inline unsigned int GetEntityVersion(EntityID id)
{
  // Cast to a 32 bit int to get our version number (loosing the top 32 bits)
  return (unsigned int)id;
}
// Checks if the EntityID has not been deleted
inline bool IsEntityValid(EntityID id)
{
  // Check if the index is our invalid index
  return (id >> 32) != (unsigned int)(-1);
}

#define INVALID_ENTITY CreateEntityId((unsigned int)(-1), 0)

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
  EntityID NewEntity()
  {
    // std::vector::size runs in constant time.
    if (!freeIndices.empty())
    {
      unsigned int newIndex = freeIndices.back();
      freeIndices.pop_back();
      // Takes in index and incremented EntityVersion at that index
      EntityID newID = CreateEntityId(newIndex, GetEntityVersion(entities[newIndex].id));
      entities[newIndex].id = newID;
      return entities[newIndex].id;
    }
    entities.push_back({ CreateEntityId((unsigned int)(entities.size()), 0), ComponentMask() });
    return entities.back().id;
  }

  // Removes a given entity from the scene and signals to the scene the free space that was left behind
  void DestroyEntity(EntityID id)
  {
    // Increments EntityVersion at the deleted index
    EntityID newID = CreateEntityId((unsigned int)(-1), GetEntityVersion(id) + 1);
    entities[GetEntityIndex(id)].id = newID;
    entities[GetEntityIndex(id)].mask.reset(); 
    freeIndices.push_back(GetEntityIndex(id));
  }

  // Removes a component from the entity with the given EntityID
  // if the EntityID is not already removed.
  template<typename T>
  void Remove(EntityID id)
  {
    // ensures you're not accessing an entity that has been deleted
    if (entities[GetEntityIndex(id)].id != id) 
      return;

    int componentId = GetId<T>();
    // Finds location of component data within the entity component pool and 
    // resets, thus removing the component from the entity
    entities[GetEntityIndex(id)].mask.reset(componentId);
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
    T* pComponent = new (componentPools[componentId]->get(GetEntityIndex(id))) T();

    entities[GetEntityIndex(id)].mask.set(componentId);
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
    if (!entities[GetEntityIndex(id)].mask.test(componentId))
      return nullptr;

    T* pComponent = static_cast<T*>(componentPools[componentId]->get(GetEntityIndex(id)));
    return pComponent;
  }
};

// Helps with iterating through a given scene
template<typename... ComponentTypes>
struct SceneView
{
  SceneView(Scene& scene)  : pScene(&scene) 
  {
    if (sizeof...(ComponentTypes) == 0)
    {
      all = true;
    }
    else
    {
      // Unpack the template parameters into an initializer list
      int componentIds[] = { 0, GetId<ComponentTypes>() ... };
      for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
        componentMask.set(componentIds[i]);
    }
  }

  struct Iterator
  {
    Iterator(Scene* pScene, unsigned int index, ComponentMask mask, bool all) 
    : pScene(pScene), index(index), mask(mask), all(all) {}

    // give back the entityID we're currently at
    EntityID operator*() const{
      return pScene->entities[index].id; 
    }

    // Compare two iterators
    bool operator==(const Iterator& other) const{
      return index == other.index || index == pScene->entities.size();
    }
    bool operator!=(const Iterator& other) const{
      return index != other.index && index != pScene->entities.size();
    }

    bool ValidIndex(){
      return
      // It's a valid entity ID
      IsEntityValid(pScene->entities[index].id) &&
      // It has the correct component mask
      (all || mask == (mask & pScene->entities[index].mask));
    }

    // Move the iterator forward
    Iterator& operator++(){
      do
      {
        index++;
      } while (index < pScene->entities.size() && !ValidIndex());
      return *this;
    }
    unsigned int index;
    Scene* pScene;
    ComponentMask mask;
    bool all{ false };
  };

  // Give an iterator to the beginning of this view
  const Iterator begin() const{
    int firstIndex = 0;
    while (firstIndex < pScene->entities.size() &&
      (componentMask != (componentMask & pScene->entities[firstIndex].mask) 
        || !IsEntityValid(pScene->entities[firstIndex].id))) 
    {
      firstIndex++;
    }
    return Iterator(pScene, firstIndex, componentMask, all);
  }

    // Give an iterator to the end of this view 
  const Iterator end() const{
    return Iterator(pScene, (unsigned int)(pScene->entities.size()), componentMask, all);
  }

  Scene* pScene{ nullptr };
  ComponentMask componentMask;
  bool all{ false };
};

int ecsDemo() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "ECS Tester");


  Scene scene;

  EntityID newEnt = scene.NewEntity();
  TransformComponent* transComp = scene.Assign<TransformComponent>(newEnt);

  sf::Clock deltaClock;

  transComp->x_pos = 100;
  transComp->y_pos = 100;

  // these can be turned into components, currently they're here for proof of concept
  // If you're seeing this, it means I haven't turned them into components yet
  // because some architecture questions need to be considered
  float ballAcceleration = 98;
  float ballMass = 5;

  // run the program as long as the window is open
  while (window.isOpen())
  {
    // Restarts the clock to get the change in time between frames
    // used for framerate-independent physics calculations
    sf::Time dt = deltaClock.restart();
    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;

    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    } 
    // clear the window with black color
    window.clear(sf::Color::Black);

    // draw everything here...
    sf::CircleShape shape(50);

    // set the shape color to green
    shape.setFillColor(sf::Color(100, 250, 50));

    // set the absolute position of the shape to the transform component
    shape.setPosition(transComp->x_pos, transComp->y_pos);

    window.draw(shape);



    // Modifies the transform component
    // Setting this after the frame has been drawn is relatively arbitrary at the moment
    // As we develop the architecture of the system more, this will change.
    // e.g if we want to have physics calculations happen before a frame is drawn.
    transComp->y_pos += ballAcceleration * dt.asSeconds(); 
                                          // we multiply by dt.asSeconds() to make it framerate-independent
                                          // e.g if two seconds passed between frames, we multiply the distance
                                          // the ball should've travelled in a frame, times 2.
    ballAcceleration += ballMass * ballAcceleration * dt.asSeconds();
    printf("%f\n", transComp->y_pos);


    // end the current frame
    window.display();

  }

  return 0;
}

// Scans for collision of a single component
// and edits trajectory of ball otherwise
void scanCollision(CircleCollider* checkCollider, Rigidbody* accessRigid, TransformComponent* accessTransform, Scene& accessScene){
  for (EntityID ent : SceneView<TransformComponent, Rigidbody, CircleCollider>(accessScene))
  {
    TransformComponent* t = accessScene.Get<TransformComponent>(ent);
    Rigidbody* rb = accessScene.Get<Rigidbody>(ent);
    CircleCollider* cc = accessScene.Get<CircleCollider>(ent);
    if(rb != accessRigid){
      double diffX = t->x_pos - accessTransform->x_pos;
      double diffY = t->y_pos - accessTransform->y_pos;
      double distance = sqrt(diffX * diffX + diffY * diffY);
      if(distance < cc->radius + checkCollider->radius){
        double normX = diffX/distance;
        double normY = diffY/distance;
        double thisSpeedMag = -sqrt(accessRigid->v_x * accessRigid->v_x + accessRigid->v_y * accessRigid->v_y);
        accessRigid->v_x = normX * thisSpeedMag;
        accessRigid->v_y = normY * thisSpeedMag;
        double speedMag = sqrt(rb->v_x * rb->v_x + rb->v_y * rb->v_y);
        rb->v_x = normX * speedMag;
        rb->v_y = normY * speedMag;
      }
    }
  }
// Generates a random float in the inclusive range of the two given
// floats.
float RandInBetween(float LO, float HI)
{
  // From https://stackoverflow.com/questions/686353/random-float-number-generation
  return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

int main() {
  srand (static_cast <unsigned> (time(0)));
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BALLS!");
  Scene scene;

  // Instantiate all the balls.
  for (u32 i = 0; i < NUM_BALLS; i++)
    {
      EntityID ball = scene.NewEntity();
      TransformComponent* pBallTransform = scene.Assign<TransformComponent>(ball);
      Rigidbody* pBallRb = scene.Assign<Rigidbody>(ball);
      CircleCollider* pBallCC = scene.Assign<CircleCollider>(ball);
      float radius = BALL_RADIUS;

      pBallTransform->x_pos = RandInBetween(radius, WINDOW_WIDTH - radius);
      pBallTransform->y_pos = RandInBetween(radius, WINDOW_HEIGHT - radius);
      pBallRb->v_x = RandInBetween(0.1, 0.5);
      pBallRb->v_y = RandInBetween(0.1, 0.5);
      pBallCC->radius = radius;
    }
  
  // run the program as long as the window is open
  while (window.isOpen())
  {
    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;

    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    } 
    // clear the window with black color
    window.clear(sf::Color::Black);

    for (EntityID ent : SceneView<TransformComponent, Rigidbody, CircleCollider>(scene))
      {
        TransformComponent* t = scene.Get<TransformComponent>(ent);
        Rigidbody* rb = scene.Get<Rigidbody>(ent);
        CircleCollider* cc = scene.Get<CircleCollider>(ent);
        float radius = cc->radius;

        // Not framerate independent for simpler collision logic.
        t->x_pos += rb->v_x;
        t->y_pos += rb->v_y;

        // Collision check x-axis
        if ((t->x_pos - radius) < 0 || (t->x_pos + radius) > WINDOW_WIDTH)
          {
            rb->v_x *= -1;
          }

        // Collision check y-axis
        if ((t->y_pos - radius) < 0 || (t->y_pos + radius) > WINDOW_HEIGHT)
          {
            rb->v_y *= -1;
          }

        scanCollision(cc, rb, t, scene);
        // Render to window
        sf::CircleShape shape(radius);
        shape.setFillColor(sf::Color(100, 250, 50));

        shape.setPosition(t->x_pos, t->y_pos);

        window.draw(shape);
      }
    
    // end the current frame
    window.display();

  }
}

