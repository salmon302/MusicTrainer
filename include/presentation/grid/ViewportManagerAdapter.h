#pragma once

#include "IViewportController.h"
#include "presentation/ViewportManager.h"
#include <memory>

namespace MusicTrainer::presentation::grid {

/**
 * @brief Adapter class that wraps ViewportManager and implements IViewportController
 * 
 * This adapter allows the existing ViewportManager implementation to be used
 * with the new grid interfaces, facilitating incremental migration.
 */
class ViewportManagerAdapter : public IViewportController {
public:
    explicit ViewportManagerAdapter(MusicTrainer::presentation::ViewportManager* viewportManager);
    ~ViewportManagerAdapter() override = default;
    
    // IViewportController implementation
    void setViewportBounds(const Rectangle& bounds) override;
    Rectangle getViewportBounds() const override;
    const ViewportState& getViewportState() const override;
    void updateViewSize(float width, float height) override;
    void updateScrollPosition(float x, float y) override;
    void updateZoomLevel(float zoom) override;
    void expandGrid(GridDirection direction, int amount) override;
    void collapseGrid(GridDirection direction) override;
    bool canCollapse(GridDirection direction) const override;
    std::optional<GridDirection> shouldExpand() const override;
    const LoadingBoundaries& getLoadingBoundaries() const override;
    void setLoadingBoundaries(const LoadingBoundaries& boundaries) override;
    void compactUnusedRegions() override;
    std::pair<int, int> mapToMusicalSpace(float screenX, float screenY) const override;
    std::pair<float, float> mapFromMusicalSpace(int position, int pitch) const override;
    void setPosition(float x, float y) override;
    std::pair<float, float> getPosition() const override;
    void setZoom(float scale) override;
    float getZoom() const override;
    void resize(int width, int height) override;
    std::pair<int, int> getDimensions() const override;
    GridCoordinate screenToGrid(float screenX, float screenY) const override;
    std::pair<float, float> gridToScreen(const GridCoordinate& gridCoord) const override;
    GridRegion getVisibleRegion() const override;
    
    // Access to the wrapped ViewportManager
    MusicTrainer::presentation::ViewportManager* getViewportManager() const { return m_viewportManager; }
    
private:
    MusicTrainer::presentation::ViewportManager* m_viewportManager;
    mutable ViewportState m_cachedState;
    mutable LoadingBoundaries m_cachedBoundaries;
    
    // Helper methods for coordinate conversion
    QRectF rectangleToQRectF(const Rectangle& rect) const;
    Rectangle qRectFToRectangle(const QRectF& rect) const;
    QPointF pointToQPointF(float x, float y) const;
    std::pair<float, float> qPointFToPoint(const QPointF& point) const;
};

} // namespace MusicTrainer::presentation::grid