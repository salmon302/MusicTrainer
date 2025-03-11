#pragma once

#include "GridTypes.h"
#include "SpatialIndex.h"

namespace MusicTrainer::presentation::grid {

/**
 * @brief Manager for grid memory optimization and cleanup
 */
class GridMemoryManager {
public:
    struct MemoryConfig {
        float bufferZoneSize;  // Buffer zone multiplier
        size_t maxMemoryUsage;    // Maximum memory usage in bytes (0 = unlimited)
        int cleanupInterval;    // Number of operations between cleanup passes
        
        MemoryConfig()
            : bufferZoneSize(1.5f)
            , maxMemoryUsage(0)
            , cleanupInterval(1000)
        {}
    };

    explicit GridMemoryManager(SpatialIndex& index, MemoryConfig config = MemoryConfig());

    // Memory management operations
    void setActiveRegion(const GridRegion& region);
    void reportOperation();
    void forceCleanup();

    // Configuration
    void setMemoryConfig(const MemoryConfig& config);
    const MemoryConfig& getMemoryConfig() const;
    size_t getCurrentMemoryUsage() const;

private:
    SpatialIndex& m_index;
    MemoryConfig m_config;
    GridRegion m_activeRegion;
    int m_operationCount{0};

    void cleanup();
    GridRegion calculateBufferZone() const;
};

} // namespace MusicTrainer::presentation::grid