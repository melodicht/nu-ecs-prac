#ifndef ECSCONSTS_HPP
#define ECSCONSTS_HPP

#include <iostream>
#include <vector>
#include <bitset>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint64_t EntityID;
static const uint32_t MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
static const uint32_t MAX_ENTITIES = 256;

/*
* ID FUNCTIONALITY 
*/

// Allows the storage of EntityID's such that they store both the Index and their Version number
// This allows for deletion without overlapping slots.
// Index and Version number are both 32 bit unsigned ints that will be combined into EntityID.
struct ECSConsts{

    static inline EntityID CreateEntityId(unsigned int index, unsigned int version)
    {
        // Shift the index up 32, and put the version in the bottom
        return ((EntityID)index << 32) | ((EntityID)version);
    }
    // This should represent the index an element has within the "entities" vector inside of a Scene
    static inline unsigned int GetEntityIndex(EntityID id)
    {
        // Shift down 32 so we lose the version and get our index
        return id >> 32;
    }

    static inline unsigned int GetEntityVersion(EntityID id)
    {
        // Cast to a 32 bit int to get our version number (loosing the top 32 bits)
        return (unsigned int)id;
    }
    // Checks if the EntityID has not been deleted
    static inline bool IsEntityValid(EntityID id)
    {
        // Check if the index is our invalid index
        return (id >> 32) != (unsigned int)(-1);
    }

    // Gets a unique identifier for the given component T, which is
    // guaranteed to be different from previous outputs for only the first
    // 2^64 unsigned ints. 
    static u64 s_componentCounter;
    template <class T>
    static int GetId()
    {
        static u64 s_componentId = s_componentCounter++;
        return s_componentId;
    }
};

#endif