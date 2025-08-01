#pragma once

#include <cstdint>

typedef uint32_t ShadowID;

// Represents a map meant
// 
template <typename GPUSideType, typename CPUSideType>
class WGPUBackendDynamicShadowVectorMap {
    std::pair<

    // Extracts ShadowID from vector map types
    virtual ShadowID GetCPUShadowID() = 0;
    virtual ShadowID GetGPUShadowID() = 0;
};