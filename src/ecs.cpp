#include <vector>
#include <bitset>

//////////////// COMPONENTS ////////////////

// Gets a unique identifier for the given component T, which is
// guaranteed to be different from previous outputs for only the first
// 2^64 u32s.
u64 s_componentCounter;

template<class T>
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

/*
 * TYPE DEFINITIONS AND CONSTANTS
 */

typedef u64 EntityID;
constexpr u32 MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
constexpr u32 MAX_ENTITIES = 32768;

/*
 * ID FUNCTIONALITY
 */

// Allows the storage of EntityID's such that they store both the Index and their Version number
// This allows for deletion without overlapping slots.
// Index and Version number are both u32s that will be combined into EntityID.

inline EntityID CreateEntityId(u32 index, u32 version)
{
    // Shift the index up 32, and put the version in the bottom
    return ((EntityID) index << 32) | ((EntityID) version);
}

// This should represent the index an element has within the "entities" vector inside a Scene
inline u32 GetEntityIndex(EntityID id)
{
    // Shift down 32 so we lose the version and get our index
    return id >> 32;
}

inline u32 GetEntityVersion(EntityID id)
{
    // Cast to a 32 bit int to get our version number (losing the top 32 bits)
    return (u32) id;
}

// Checks if the EntityID has not been deleted
inline bool IsEntityValid(EntityID id)
{
    // Check if the index is our invalid index
    return (id >> 32) != (u32) (-1);
}

#define INVALID_ENTITY CreateEntityId((u32)(-1), 0)

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
    inline void *get(size_t index)
    {
        // looking up the component at the desired index
        return pData + index * elementSize;
    }

    u8 *pData{nullptr};
    size_t elementSize{0};
};

struct Scene;

// A system in our ECS, which defines operations on a subset of
// entities, using scene view.
class System
{
public:
    virtual void OnStart(Scene *scene) {};
    virtual void OnUpdate(Scene *scene) {};
    virtual ~System() = default;
};

/*
 * SCENE DEFINITION + FUNCTIONALITY
 */

// Each component has its own memory pool, to have good memory
// locality. An entity's ID is the index into its own component in the
// component pool.
struct Scene
{
    struct EntityEntry
    {
        EntityID id; // though redundent with index in vector, required
        // for deleting entities,
        ComponentMask mask;
    };

    std::vector<EntityEntry> entities;
    std::vector<ComponentPool *> componentPools;
    std::vector<u32> freeIndices;
    std::vector<System *> systems;

    void AddSystem(System *sys)
    {
        systems.push_back(sys);
    }

    void InitSystems()
    {
        for (System *sys : systems)
        {
            sys->OnStart(this);
        }
    }

    void UpdateSystems()
    {
        for (System *sys: systems)
        {
            sys->OnUpdate(this);
        }
    }

    // Adds a new entity to this vector of entities, and returns its
    // ID. Can only support 2^64 entities without ID conflicts.
    EntityID NewEntity()
    {
        // std::vector::size runs in constant time.
        if (!freeIndices.empty())
        {
            u32 newIndex = freeIndices.back();
            freeIndices.pop_back();
            // Takes in index and incremented EntityVersion at that index
            EntityID newID = CreateEntityId(newIndex, GetEntityVersion(entities[newIndex].id));
            entities[newIndex].id = newID;
            return entities[newIndex].id;
        }
        entities.push_back({CreateEntityId((u32) (entities.size()), 0), ComponentMask()});
        return entities.back().id;
    }

    // Removes a given entity from the scene and signals to the scene the free space that was left behind
    void DestroyEntity(EntityID id)
    {
        // Increments EntityVersion at the deleted index
        EntityID newID = CreateEntityId((u32) (-1), GetEntityVersion(id) + 1);
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
    T *Assign(EntityID id)
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
        T *pComponent = new(componentPools[componentId]->get(GetEntityIndex(id))) T();

        entities[GetEntityIndex(id)].mask.set(componentId);
        return pComponent;
    }

    // Returns the pointer to the component instance on the entity
    // associated with the given ID in this vector of entities, with the
    // given component type. Returns nullptr if that entity doesn't have
    // the given component type.
    template<typename T>
    T *Get(EntityID id)
    {
        int componentId = GetId<T>();
        if (!entities[GetEntityIndex(id)].mask.test(componentId))
            return nullptr;

        T *pComponent = static_cast<T *>(componentPools[componentId]->get(GetEntityIndex(id)));
        return pComponent;
    }
};

// Helps with iterating through a given scene
template<typename... ComponentTypes>
struct SceneView
{
    SceneView(Scene &scene) : pScene(&scene)
    {
        if (sizeof...(ComponentTypes) == 0)
        {
            all = true;
        }
        else
        {
            // Unpack the template parameters into an initializer list
            int componentIds[] = {0, GetId<ComponentTypes>()...};
            for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
                componentMask.set(componentIds[i]);
        }
    }

    struct Iterator
    {
        Iterator(Scene *pScene, u32 index, ComponentMask mask, bool all)
            : pScene(pScene), index(index), mask(mask), all(all) {}

        // give back the entityID we're currently at
        EntityID operator*() const
        {
            return pScene->entities[index].id;
        }

        // Compare two iterators
        bool operator==(const Iterator &other) const
        {
            return index == other.index || index == pScene->entities.size();
        }

        bool operator!=(const Iterator &other) const
        {
            return index != other.index && index != pScene->entities.size();
        }

        bool ValidIndex()
        {
            return
                    // It's a valid entity ID
                    IsEntityValid(pScene->entities[index].id) &&
                    // It has the correct component mask
                    (all || mask == (mask & pScene->entities[index].mask));
        }

        // Move the iterator forward
        Iterator &operator++()
        {
            do
            {
                index++;
            } while (index < pScene->entities.size() && !ValidIndex());
            return *this;
        }

        u32 index;
        Scene *pScene;
        ComponentMask mask;
        bool all{false};
    };

    // Give an iterator to the beginning of this view
    const Iterator begin() const
    {
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
    const Iterator end() const
    {
        return Iterator(pScene, (u32) (pScene->entities.size()), componentMask, all);
    }

    Scene *pScene{nullptr};
    ComponentMask componentMask;
    bool all{false};
};
