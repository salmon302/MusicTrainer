#include "presentation/grid/SpatialIndex.h"
#include <cmath>

namespace MusicTrainer::presentation::grid {

SpatialIndex::SpatialIndex(int chunkSize)
    : m_chunkSize(chunkSize)
{
}

void SpatialIndex::insert(const GridCoordinate& coord, const GridCellValue& value) {
    auto [chunkX, chunkY] = getChunkCoords(coord);
    auto& chunk = m_chunks[chunkX][chunkY];
    chunk.cells[coord.position][coord.pitch] = value;
}

bool SpatialIndex::remove(const GridCoordinate& coord) {
    auto [chunkX, chunkY] = getChunkCoords(coord);
    auto chunkIt = m_chunks.find(chunkX);
    if (chunkIt == m_chunks.end()) return false;

    auto subChunkIt = chunkIt->second.find(chunkY);
    if (subChunkIt == chunkIt->second.end()) return false;

    auto& chunk = subChunkIt->second;
    auto posIt = chunk.cells.find(coord.position);
    if (posIt == chunk.cells.end()) return false;

    auto& pitchMap = posIt->second;
    auto pitchIt = pitchMap.find(coord.pitch);
    if (pitchIt == pitchMap.end()) return false;

    pitchMap.erase(pitchIt);
    if (pitchMap.empty()) {
        chunk.cells.erase(posIt);
    }
    return true;
}

bool SpatialIndex::contains(const GridCoordinate& coord) const {
    auto [chunkX, chunkY] = getChunkCoords(coord);
    auto chunkIt = m_chunks.find(chunkX);
    if (chunkIt == m_chunks.end()) return false;

    auto subChunkIt = chunkIt->second.find(chunkY);
    if (subChunkIt == chunkIt->second.end()) return false;

    const auto& chunk = subChunkIt->second;
    auto posIt = chunk.cells.find(coord.position);
    if (posIt == chunk.cells.end()) return false;

    return posIt->second.find(coord.pitch) != posIt->second.end();
}

GridCellValue SpatialIndex::getValue(const GridCoordinate& coord) const {
    auto [chunkX, chunkY] = getChunkCoords(coord);
    return m_chunks.at(chunkX).at(chunkY).cells.at(coord.position).at(coord.pitch);
}

std::vector<GridCell> SpatialIndex::getInRegion(const GridRegion& region) const {
    std::vector<GridCell> result;
    auto chunks = getChunksInRegion(region);

    for (const auto& [chunkX, chunkY] : chunks) {
        auto chunkIt = m_chunks.find(chunkX);
        if (chunkIt == m_chunks.end()) continue;

        auto subChunkIt = chunkIt->second.find(chunkY);
        if (subChunkIt == chunkIt->second.end()) continue;

        const auto& chunk = subChunkIt->second;
        for (const auto& [pos, pitchMap] : chunk.cells) {
            if (pos < region.startPosition || pos >= region.endPosition) continue;

            for (const auto& [pitch, value] : pitchMap) {
                if (pitch < region.minPitch || pitch >= region.maxPitch) continue;
                result.push_back({{pos, pitch}, value});
            }
        }
    }

    return result;
}

void SpatialIndex::clear() {
    m_chunks.clear();
}

void SpatialIndex::insertBatch(const std::vector<GridCell>& cells) {
    for (const auto& cell : cells) {
        insert(cell.coord, cell.value);
    }
}

void SpatialIndex::removeBatch(const std::vector<GridCoordinate>& coords) {
    for (const auto& coord : coords) {
        remove(coord);
    }
}

void SpatialIndex::compact(const GridRegion& activeRegion) {
    auto chunks = getChunksInRegion(activeRegion);
    std::unordered_map<int, std::unordered_map<int, Chunk>> newChunks;

    for (const auto& [chunkX, chunkY] : chunks) {
        auto chunkIt = m_chunks.find(chunkX);
        if (chunkIt == m_chunks.end()) continue;

        auto subChunkIt = chunkIt->second.find(chunkY);
        if (subChunkIt == chunkIt->second.end()) continue;

        newChunks[chunkX][chunkY] = subChunkIt->second;
    }

    m_chunks = std::move(newChunks);
}

size_t SpatialIndex::getMemoryUsage() const {
    size_t totalSize = sizeof(*this);
    
    for (const auto& [_, subMap] : m_chunks) {
        totalSize += sizeof(subMap);
        for (const auto& [_, chunk] : subMap) {
            totalSize += sizeof(chunk);
            for (const auto& [_, pitchMap] : chunk.cells) {
                totalSize += sizeof(pitchMap) + (pitchMap.size() * sizeof(GridCellValue));
            }
        }
    }

    return totalSize;
}

std::pair<int, int> SpatialIndex::getChunkCoords(const GridCoordinate& coord) const {
    return {
        static_cast<int>(std::floor(static_cast<float>(coord.position) / m_chunkSize)),
        static_cast<int>(std::floor(static_cast<float>(coord.pitch) / m_chunkSize))
    };
}

std::vector<std::pair<int, int>> SpatialIndex::getChunksInRegion(const GridRegion& region) const {
    std::vector<std::pair<int, int>> chunks;
    int startChunkX = static_cast<int>(std::floor(static_cast<float>(region.startPosition) / m_chunkSize));
    int endChunkX = static_cast<int>(std::floor(static_cast<float>(region.endPosition) / m_chunkSize));
    int startChunkY = static_cast<int>(std::floor(static_cast<float>(region.minPitch) / m_chunkSize));
    int endChunkY = static_cast<int>(std::floor(static_cast<float>(region.maxPitch) / m_chunkSize));

    for (int x = startChunkX; x <= endChunkX; ++x) {
        for (int y = startChunkY; y <= endChunkY; ++y) {
            chunks.emplace_back(x, y);
        }
    }

    return chunks;
}

} // namespace MusicTrainer::presentation::grid