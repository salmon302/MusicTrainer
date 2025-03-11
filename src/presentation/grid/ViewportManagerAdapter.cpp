#include "presentation/grid/ViewportManagerAdapter.h"
#include <QGraphicsView>
#include <QRectF>

namespace MusicTrainer::presentation::grid {

// Helper function to convert grid GridDirection to presentation GridDirection
static MusicTrainer::presentation::GridDirection convertDirection(GridDirection dir) {
    switch (dir) {
        case GridDirection::Up:
            return MusicTrainer::presentation::GridDirection::Up;
        case GridDirection::Down:
            return MusicTrainer::presentation::GridDirection::Down;
        case GridDirection::Right:
            return MusicTrainer::presentation::GridDirection::Right;
        default:
            return MusicTrainer::presentation::GridDirection::Right;
    }
}

// Helper function to convert presentation GridDirection to grid GridDirection
static GridDirection convertDirection(MusicTrainer::presentation::GridDirection dir) {
    switch (dir) {
        case MusicTrainer::presentation::GridDirection::Up:
            return GridDirection::Up;
        case MusicTrainer::presentation::GridDirection::Down:
            return GridDirection::Down;
        case MusicTrainer::presentation::GridDirection::Right:
            return GridDirection::Right;
        default:
            return GridDirection::Right;
    }
}

ViewportManagerAdapter::ViewportManagerAdapter(MusicTrainer::presentation::ViewportManager* viewportManager)
    : m_viewportManager(viewportManager)
{
    // Initialize cached state from the ViewportManager
    auto vmState = m_viewportManager->getViewportState();
    m_cachedState = {
        qRectFToRectangle(vmState.visibleArea),
        vmState.zoomLevel,
        {static_cast<float>(vmState.scrollPosition.x()), static_cast<float>(vmState.scrollPosition.y())},
        vmState.preserveOctaveExpansion
    };
    
    // Initialize cached boundaries
    auto vmBoundaries = m_viewportManager->getLoadingBoundaries();
    m_cachedBoundaries = {
        vmBoundaries.horizontalBuffer,
        vmBoundaries.verticalBuffer
    };
}

void ViewportManagerAdapter::setViewportBounds(const Rectangle& bounds) {
    m_viewportManager->setViewportBounds(rectangleToQRectF(bounds));
}

Rectangle ViewportManagerAdapter::getViewportBounds() const {
    return qRectFToRectangle(m_viewportManager->getViewportBounds());
}

const ViewportManagerAdapter::ViewportState& ViewportManagerAdapter::getViewportState() const {
    // Update cached state from the ViewportManager
    auto vmState = m_viewportManager->getViewportState();
    m_cachedState = {
        qRectFToRectangle(vmState.visibleArea),
        vmState.zoomLevel,
        {static_cast<float>(vmState.scrollPosition.x()), static_cast<float>(vmState.scrollPosition.y())},
        vmState.preserveOctaveExpansion
    };
    
    return m_cachedState;
}

void ViewportManagerAdapter::updateViewSize(float width, float height) {
    m_viewportManager->updateViewSize(QSize(static_cast<int>(width), static_cast<int>(height)));
}

void ViewportManagerAdapter::updateScrollPosition(float x, float y) {
    m_viewportManager->updateScrollPosition(QPointF(x, y));
}

void ViewportManagerAdapter::updateZoomLevel(float zoom) {
    m_viewportManager->updateZoomLevel(zoom);
}

void ViewportManagerAdapter::expandGrid(GridDirection direction, int amount) {
    m_viewportManager->expandGrid(convertDirection(direction), amount);
}

void ViewportManagerAdapter::collapseGrid(GridDirection direction) {
    m_viewportManager->collapseGrid(convertDirection(direction));
}

bool ViewportManagerAdapter::canCollapse(GridDirection direction) const {
    return m_viewportManager->canCollapse(convertDirection(direction));
}

std::optional<GridDirection> ViewportManagerAdapter::shouldExpand() const {
    auto result = m_viewportManager->shouldExpand();
    if (result.has_value()) {
        return convertDirection(result.value());
    }
    return std::nullopt;
}

const ViewportManagerAdapter::LoadingBoundaries& ViewportManagerAdapter::getLoadingBoundaries() const {
    // Update cached boundaries from the ViewportManager
    auto vmBoundaries = m_viewportManager->getLoadingBoundaries();
    m_cachedBoundaries = {
        vmBoundaries.horizontalBuffer,
        vmBoundaries.verticalBuffer
    };
    
    return m_cachedBoundaries;
}

void ViewportManagerAdapter::setLoadingBoundaries(const LoadingBoundaries& boundaries) {
    MusicTrainer::presentation::ViewportManager::LoadingBoundaries vmBoundaries{
        boundaries.horizontalBuffer,
        boundaries.verticalBuffer
    };
    
    m_viewportManager->setLoadingBoundaries(vmBoundaries);
}

void ViewportManagerAdapter::compactUnusedRegions() {
    m_viewportManager->compactUnusedRegions();
}

std::pair<int, int> ViewportManagerAdapter::mapToMusicalSpace(float screenX, float screenY) const {
    // Get current viewport state and dimensions
    auto state = m_viewportManager->getViewportState();
    auto bounds = m_viewportManager->getViewportBounds();
    
    // Calculate the offset due to scrolling
    float offsetX = state.scrollPosition.x();
    float offsetY = state.scrollPosition.y();
    
    // Apply zoom factor
    float zoomedX = screenX / state.zoomLevel;
    float zoomedY = screenY / state.zoomLevel;
    
    // Add scroll offset to get scene coordinates
    float sceneX = zoomedX + offsetX;
    float sceneY = zoomedY + offsetY;
    
    // Convert scene coordinates to musical space
    // In the traditional grid, GRID_UNIT defines horizontal spacing and NOTE_HEIGHT defines vertical
    constexpr float GRID_UNIT = 40.0f;  // Match the constant in NoteGrid.h
    constexpr float NOTE_HEIGHT = 15.0f; // Match the constant in NoteGrid.h
    
    int position = static_cast<int>(sceneX / GRID_UNIT);
    int pitch = static_cast<int>(sceneY / NOTE_HEIGHT);
    
    return {position, pitch};
}

std::pair<float, float> ViewportManagerAdapter::mapFromMusicalSpace(int position, int pitch) const {
    // Convert musical coordinates to scene coordinates
    constexpr float GRID_UNIT = 40.0f;  // Match the constant in NoteGrid.h
    constexpr float NOTE_HEIGHT = 15.0f; // Match the constant in NoteGrid.h
    
    float sceneX = position * GRID_UNIT;
    float sceneY = pitch * NOTE_HEIGHT;
    
    // Get current viewport state
    auto state = m_viewportManager->getViewportState();
    
    // Calculate screen coordinates by accounting for scroll position and zoom
    float screenX = (sceneX - state.scrollPosition.x()) * state.zoomLevel;
    float screenY = (sceneY - state.scrollPosition.y()) * state.zoomLevel;
    
    return {screenX, screenY};
}

void ViewportManagerAdapter::setPosition(float x, float y) {
    // This is a placeholder implementation since ViewportManager doesn't have a direct equivalent
    // In a real implementation, you would need to convert the position to a scroll position
    // and update the ViewportManager
    
    updateScrollPosition(x, y);
}

std::pair<float, float> ViewportManagerAdapter::getPosition() const {
    // This is a placeholder implementation since ViewportManager doesn't have a direct equivalent
    // In a real implementation, you would need to get the scroll position from the ViewportManager
    
    auto vmState = m_viewportManager->getViewportState();
    return {vmState.scrollPosition.x(), vmState.scrollPosition.y()};
}

void ViewportManagerAdapter::setZoom(float scale) {
    m_viewportManager->updateZoomLevel(scale);
}

float ViewportManagerAdapter::getZoom() const {
    return m_viewportManager->getViewportState().zoomLevel;
}

void ViewportManagerAdapter::resize(int width, int height) {
    m_viewportManager->updateViewSize(QSize(width, height));
}

std::pair<int, int> ViewportManagerAdapter::getDimensions() const {
    // This is a placeholder implementation since ViewportManager doesn't have a direct equivalent
    // In a real implementation, you would need to get the view size from the ViewportManager
    
    auto vmState = m_viewportManager->getViewportState();
    return {
        static_cast<int>(vmState.visibleArea.width()),
        static_cast<int>(vmState.visibleArea.height())
    };
}

GridCoordinate ViewportManagerAdapter::screenToGrid(float screenX, float screenY) const {
    auto [position, pitch] = mapToMusicalSpace(screenX, screenY);
    
    // Ensure coordinates are within valid range
    auto bounds = m_viewportManager->getViewportBounds();
    position = std::max(static_cast<int>(bounds.left()), 
                      std::min(static_cast<int>(bounds.right() - 1), position));
    pitch = std::max(static_cast<int>(bounds.top()), 
                    std::min(static_cast<int>(bounds.bottom() - 1), pitch));
    
    return {position, pitch};
}

std::pair<float, float> ViewportManagerAdapter::gridToScreen(const GridCoordinate& gridCoord) const {
    return mapFromMusicalSpace(gridCoord.position, gridCoord.pitch);
}

GridRegion ViewportManagerAdapter::getVisibleRegion() const {
    auto bounds = m_viewportManager->getViewportBounds();
    
    return {
        static_cast<int>(bounds.left()),
        static_cast<int>(bounds.right()),
        static_cast<int>(bounds.top()),
        static_cast<int>(bounds.bottom())
    };
}

QRectF ViewportManagerAdapter::rectangleToQRectF(const Rectangle& rect) const {
    return QRectF(rect.x, rect.y, rect.width, rect.height);
}

Rectangle ViewportManagerAdapter::qRectFToRectangle(const QRectF& rect) const {
    return {
        static_cast<float>(rect.x()),
        static_cast<float>(rect.y()),
        static_cast<float>(rect.width()),
        static_cast<float>(rect.height())
    };
}

QPointF ViewportManagerAdapter::pointToQPointF(float x, float y) const {
    return QPointF(x, y);
}

std::pair<float, float> ViewportManagerAdapter::qPointFToPoint(const QPointF& point) const {
    return {static_cast<float>(point.x()), static_cast<float>(point.y())};
}

} // namespace MusicTrainer::presentation::grid