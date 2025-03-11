#include "presentation/grid/CoordinateTransformer.h"
#include <algorithm>
#include <cmath>

namespace MusicTrainer::presentation::grid {

CoordinateTransformer::CoordinateTransformer(TransformConfig config)
    : m_config(std::move(config))
    , m_currentScale(m_config.baseScaleFactor)
{
}

GridCoordinate CoordinateTransformer::screenToGrid(float screenX, float screenY) const {
    float effectiveWidth = getEffectiveCellWidth();
    float effectiveHeight = getEffectiveCellHeight();

    return GridCoordinate{
        static_cast<int>(std::floor(screenX / effectiveWidth)),
        static_cast<int>(std::floor(screenY / effectiveHeight))
    };
}

std::pair<float, float> CoordinateTransformer::gridToScreen(const GridCoordinate& coord) const {
    float effectiveWidth = getEffectiveCellWidth();
    float effectiveHeight = getEffectiveCellHeight();

    return {
        coord.position * effectiveWidth,
        coord.pitch * effectiveHeight
    };
}

Rectangle CoordinateTransformer::gridRegionToScreen(const GridRegion& region) const {
    auto topLeft = gridToScreen({region.startPosition, region.minPitch});
    auto bottomRight = gridToScreen({region.endPosition, region.maxPitch});

    return Rectangle{
        topLeft.first,
        topLeft.second,
        bottomRight.first - topLeft.first,
        bottomRight.second - topLeft.second
    };
}

GridRegion CoordinateTransformer::screenRectToGrid(const Rectangle& rect) const {
    auto topLeft = screenToGrid(rect.x, rect.y);
    auto bottomRight = screenToGrid(rect.x + rect.width, rect.y + rect.height);

    return GridRegion{
        topLeft.position,
        bottomRight.position,
        topLeft.pitch,
        bottomRight.pitch
    };
}

void CoordinateTransformer::setConfig(const TransformConfig& config) {
    m_config = config;
    m_currentScale = clampScale(m_currentScale);
}

const CoordinateTransformer::TransformConfig& CoordinateTransformer::getConfig() const {
    return m_config;
}

void CoordinateTransformer::setScale(float scale) {
    m_currentScale = clampScale(scale);
}

float CoordinateTransformer::getScale() const {
    return m_currentScale;
}

float CoordinateTransformer::getCellWidth() const {
    return m_config.cellWidth;
}

float CoordinateTransformer::getCellHeight() const {
    return m_config.cellHeight;
}

float CoordinateTransformer::getEffectiveCellWidth() const {
    return m_config.cellWidth * m_currentScale;
}

float CoordinateTransformer::getEffectiveCellHeight() const {
    return m_config.cellHeight * m_currentScale;
}

float CoordinateTransformer::clampScale(float scale) const {
    return std::clamp(scale, m_config.minScale, m_config.maxScale);
}

} // namespace MusicTrainer::presentation::grid