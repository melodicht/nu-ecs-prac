#include "ComponentPool.hpp"

ComponentPool::ComponentPool(size_t elementsize)
{
    // We'll allocate enough memory to hold MAX_ENTITIES, each with element size
    elementSize = elementsize;
    pData = new u8[elementSize * MAX_ENTITIES];
}

ComponentPool::~ComponentPool()
{
    delete[] pData;
}