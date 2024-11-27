// /*
//  * COMPONENT POOL
//  */

// // Responsible for allocating contiguous memory for the components
// // such that `MAX_ENTITIES` can be stored, and components be accessed
// // via index.
// // NOTE: The memory pool is an array of bytes, as the size of one
// // component isn't known at compile time.
// struct ComponentPool
// {
//   ComponentPool(size_t elementsize)
//   {
//     // We'll allocate enough memory to hold MAX_ENTITIES, each with element size
//     elementSize = elementsize;
//     pData = new u8[elementSize * MAX_ENTITIES];
//   }

//   ~ComponentPool()
//   {
//     delete[] pData;
//   }

//   // Gets the component in this pData at the given index.
//   inline void* get(size_t index)
//   {
//     // looking up the component at the desired index
//     return pData + index * elementSize;
//   }

//   u8* pData{ nullptr };
//   size_t elementSize{ 0 };
// };