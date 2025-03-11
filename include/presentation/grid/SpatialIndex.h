#pragma once

#include <unordered_map>
#include <vector>
#include "GridTypes.h"

namespace MusicTrainer::presentation::grid {

/**
 * @brief Spatial index for efficient grid cell storage and querying
 * 
 * Uses a quad-tree like structure with a hierarchical hash map for quick
 * spatial lookups and region queries.
 */
class SpatialIndex {
public:
    // Constructor with configurable chunk size
    explicit SpatialIndex(int chunkSize = 16);

    // Core operations
    void insert(const GridCoordinate& coord, const GridCellValue& value);
    bool remove(const GridCoordinate& coord);
    bool contains(const GridCoordinate& coord) const;
    GridCellValue getValue(const GridCoordinate& coord) const;
    std::vector<GridCell> getInRegion(const GridRegion& region) const;
    void clear();

    // Batch operations
    void insertBatch(const std::vector<GridCell>& cells);
    void removeBatch(const std::vector<GridCoordinate>& coords);

    // Memory management
    void compact(const GridRegion& activeRegion);
    size_t getMemoryUsage() const;

private:
    struct Chunk {
        std::unordered_map<int, std::unordered_map<int, GridCellValue>> cells;
    };

    std::unordered_map<int, std::unordered_map<int, Chunk>> m_chunks;
    int m_chunkSize;

    // Helper methods
    std::pair<int, int> getChunkCoords(const GridCoordinate& coord) const;
    std::vector<std::pair<int, int>> getChunksInRegion(const GridRegion& region) const;
};

} // namespace MusicTrainer::presentation::grid