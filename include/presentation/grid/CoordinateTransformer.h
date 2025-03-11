#pragma once
#include <cmath>
#include "GridTypes.h"
#include "IGridModel.h"
#include "GridTypes.h"

// Simple point class to avoid Qt dependency
struct Point {
    float x, y;
    
    Point(float xPos = 0.0f, float yPos = 0.0f) : x(xPos), y(yPos) {}
};

namespace MusicTrainer::presentation::grid {

/**
 * @brief Handles coordinate transformations between screen and musical space
 */
class CoordinateTransformer {
public:
    struct TransformConfig {
        float cellWidth;      // Width of each grid cell in pixels
        float cellHeight;     // Height of each grid cell in pixels
        float baseScaleFactor; // Base scale factor for zoom
        float minScale;       // Minimum allowed scale
        float maxScale;        // Maximum allowed scale
        
        TransformConfig()
            : cellWidth(50.0f)
            , cellHeight(20.0f)
            , baseScaleFactor(1.0f)
            , minScale(0.25f)
            , maxScale(4.0f)
        {}
    };

    explicit CoordinateTransformer(TransformConfig config = TransformConfig());

    // Transform operations
    GridCoordinate screenToGrid(float screenX, float screenY) const;
    std::pair<float, float> gridToScreen(const GridCoordinate& coord) const;
    
    // Region transformations
    Rectangle gridRegionToScreen(const GridRegion& region) const;
    GridRegion screenRectToGrid(const Rectangle& rect) const;

    // Configuration
    void setConfig(const TransformConfig& config);
    const TransformConfig& getConfig() const;
    void setScale(float scale);
    float getScale() const;

    // Utility functions
    float getCellWidth() const;
    float getCellHeight() const;
    float getEffectiveCellWidth() const;
    float getEffectiveCellHeight() const;

private:
    TransformConfig m_config;
    float m_currentScale;

    float clampScale(float scale) const;
};

} // namespace MusicTrainer::presentation::grid