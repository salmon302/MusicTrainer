#include "presentation/grid/GridMemoryManager.h"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace MusicTrainer::presentation::grid {

GridMemoryManager::GridMemoryManager(SpatialIndex& index, MemoryConfig config)
    : m_index(index)
    , m_config(std::move(config))
{
    // Initialize with reasonable defaults if not provided
    if (m_config.maxMemoryUsage == 0) {
        m_config.maxMemoryUsage = 50 * 1024 * 1024; // Default to 50MB
    }
    if (m_config.cleanupInterval == 0) {
        m_config.cleanupInterval = 1000;
    }
    if (m_config.bufferZoneSize < 1.0f) {
        m_config.bufferZoneSize = 1.5f;
    }
}

void GridMemoryManager::setActiveRegion(const GridRegion& region) {
    // Check if the region has significantly changed before triggering cleanup
    bool needsCleanup = false;
    
    if (m_activeRegion.startPosition != region.startPosition ||
        m_activeRegion.endPosition != region.endPosition ||
        m_activeRegion.minPitch != region.minPitch ||
        m_activeRegion.maxPitch != region.maxPitch) {
        
        // Calculate how much the viewport has moved
        float horizontalMove = std::abs(
            (region.startPosition + region.endPosition) / 2.0f - 
            (m_activeRegion.startPosition + m_activeRegion.endPosition) / 2.0f
        );
        
        float verticalMove = std::abs(
            (region.minPitch + region.maxPitch) / 2.0f - 
            (m_activeRegion.minPitch + m_activeRegion.maxPitch) / 2.0f
        );
        
        // If significant movement, trigger cleanup
        float horizontalThreshold = (region.endPosition - region.startPosition) * 0.25f;
        float verticalThreshold = (region.maxPitch - region.minPitch) * 0.25f;
        
        if (horizontalMove > horizontalThreshold || verticalMove > verticalThreshold) {
            needsCleanup = true;
        }
    }
    
    m_activeRegion = region;
    
    if (needsCleanup) {
        cleanup();
    }
}

void GridMemoryManager::reportOperation() {
    ++m_operationCount;
    
    // Check memory usage more frequently when nearing the limit
    size_t currentUsage = getCurrentMemoryUsage();
    float usageRatio = static_cast<float>(currentUsage) / static_cast<float>(m_config.maxMemoryUsage);
    
    int adjustedInterval = m_config.cleanupInterval;
    
    // Adjust interval based on memory pressure
    if (usageRatio > 0.8f) {
        // When memory usage is high, check more frequently
        adjustedInterval = m_config.cleanupInterval / 4;
    } else if (usageRatio > 0.5f) {
        // Moderate memory usage
        adjustedInterval = m_config.cleanupInterval / 2;
    }
    
    // Perform cleanup if needed
    if (m_operationCount >= adjustedInterval) {
        cleanup();
        m_operationCount = 0;
    }
}

void GridMemoryManager::forceCleanup() {
    // Always perform a cleanup when explicitly requested
    cleanup();
}

void GridMemoryManager::setMemoryConfig(const MemoryConfig& config) {
    m_config = config;
    
    // Validate config values
    if (m_config.maxMemoryUsage == 0) {
        m_config.maxMemoryUsage = 50 * 1024 * 1024; // Default to 50MB
    }
    if (m_config.cleanupInterval == 0) {
        m_config.cleanupInterval = 1000;
    }
    if (m_config.bufferZoneSize < 1.0f) {
        m_config.bufferZoneSize = 1.5f;
    }
    
    // Force cleanup with new settings
    cleanup();
}

const GridMemoryManager::MemoryConfig& GridMemoryManager::getMemoryConfig() const {
    return m_config;
}

size_t GridMemoryManager::getCurrentMemoryUsage() const {
    return m_index.getMemoryUsage();
}

void GridMemoryManager::cleanup() {
    // Track cleanup time
    auto startTime = std::chrono::high_resolution_clock::now();
    
    size_t beforeUsage = getCurrentMemoryUsage();
    
    // Check if we're over the memory limit
    if (m_config.maxMemoryUsage > 0 && beforeUsage > m_config.maxMemoryUsage) {
        // Calculate the buffer zone for items to keep
        GridRegion bufferZone = calculateBufferZone();
        
        // Progressive cleanup - first try a smaller buffer zone if memory pressure is high
        float usageRatio = static_cast<float>(beforeUsage) / static_cast<float>(m_config.maxMemoryUsage);
        if (usageRatio > 1.5f) {
            // Under high memory pressure, use a tighter buffer zone
            int horizontalBuffer = static_cast<int>((m_activeRegion.endPosition - m_activeRegion.startPosition) * 
                                                  (1.2f - 1.0f)); // Use smaller buffer
            int verticalBuffer = static_cast<int>((m_activeRegion.maxPitch - m_activeRegion.minPitch) * 
                                                (1.2f - 1.0f));
                                                
            bufferZone = GridRegion{
                m_activeRegion.startPosition - horizontalBuffer,
                m_activeRegion.endPosition + horizontalBuffer,
                m_activeRegion.minPitch - verticalBuffer,
                m_activeRegion.maxPitch + verticalBuffer
            };
        }
        
        // Actually perform the cleanup
        m_index.compact(bufferZone);
        
        // Measure effectiveness
        size_t afterUsage = getCurrentMemoryUsage();
        float reductionPercent = 100.0f * (1.0f - static_cast<float>(afterUsage) / static_cast<float>(beforeUsage));
        
        // Log aggressive cleanup if memory is still too high
        if (afterUsage > m_config.maxMemoryUsage * 0.9f) {
            // Memory is still high, force a more aggressive cleanup with exact active region
            m_index.compact(m_activeRegion);
            
            size_t finalUsage = getCurrentMemoryUsage();
            std::cout << "WARNING: Memory usage critical. Performed aggressive cleanup: "
                      << beforeUsage / 1024 << "KB -> " << finalUsage / 1024 << "KB ("
                      << 100.0f * (1.0f - static_cast<float>(finalUsage) / static_cast<float>(beforeUsage))
                      << "% reduction)" << std::endl;
        } else if (reductionPercent > 0.5f) {
            // Only log if significant memory was freed
            std::cout << "Memory cleaned up: " << beforeUsage / 1024 << "KB -> "
                      << afterUsage / 1024 << "KB (" << reductionPercent << "% reduction)" << std::endl;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    // Log if cleanup took an unusually long time
    if (duration > 100) {
        std::cout << "WARNING: Grid memory cleanup took " << duration << "ms" << std::endl;
    }
}

GridRegion GridMemoryManager::calculateBufferZone() const {
    int horizontalBuffer = static_cast<int>((m_activeRegion.endPosition - m_activeRegion.startPosition) * 
                                          (m_config.bufferZoneSize - 1.0f));
    int verticalBuffer = static_cast<int>((m_activeRegion.maxPitch - m_activeRegion.minPitch) * 
                                        (m_config.bufferZoneSize - 1.0f));
    
    // Ensure we have at least a minimum buffer (4 measures horizontally, 1 octave vertically)
    horizontalBuffer = std::max(horizontalBuffer, 16);
    verticalBuffer = std::max(verticalBuffer, 12);
    
    return GridRegion{
        m_activeRegion.startPosition - horizontalBuffer,
        m_activeRegion.endPosition + horizontalBuffer,
        m_activeRegion.minPitch - verticalBuffer,
        m_activeRegion.maxPitch + verticalBuffer
    };
}

} // namespace MusicTrainer::presentation::grid