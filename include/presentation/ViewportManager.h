#pragma once
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <memory>
#include <optional>
#include "presentation/GridTypes.h"

QT_BEGIN_NAMESPACE
class QGraphicsView;
QT_END_NAMESPACE

namespace MusicTrainer::presentation {

class NoteGrid;

class ViewportManager {
public:
    struct ViewportState {
        QRectF visibleArea;        // Current viewport bounds in musical coordinates
        float zoomLevel;           // Current zoom level
        QPointF scrollPosition;    // Current scroll position in scene coordinates
        bool preserveOctaveExpansion = false; // Whether to maintain expanded octaves
    };

    struct LoadingBoundaries {
        float horizontalBuffer;
        float verticalBuffer;
    };

    explicit ViewportManager(NoteGrid* grid);
    ~ViewportManager();

    // Prevent copying
    ViewportManager(const ViewportManager&) = delete;
    ViewportManager& operator=(const ViewportManager&) = delete;

    // Viewport state management
    void setViewportBounds(const QRectF& bounds);
    QRectF getViewportBounds() const;
    const ViewportState& getViewportState() const;
    void updateViewSize(const QSize& size);
    void updateScrollPosition(const QPointF& scrollPos);
    void updateZoomLevel(float zoom);

    // Grid expansion methods
    void expandGrid(GridDirection direction, int amount);
    void collapseGrid(GridDirection direction);
    bool canCollapse(GridDirection direction) const;
    std::optional<GridDirection> shouldExpand() const;

    // Loading boundary management
    const LoadingBoundaries& getLoadingBoundaries() const;
    void setLoadingBoundaries(const LoadingBoundaries& boundaries);
    void compactUnusedRegions();

    // Coordinate transformation
    QPointF mapToMusicalSpace(const QPointF& screenPoint, const QGraphicsView* view) const;
    QPointF mapFromMusicalSpace(const QPointF& musicalPoint, const QGraphicsView* view) const;
    
    // Helper function for delta calculations
    QPointF calculateScaledDelta(const QPointF& screen, const QPointF& scene) const;

private:
    ViewportState m_currentState;
    ViewportState validateState(const ViewportState& state) const;
    bool updateViewportState(const ViewportState& newState);

    NoteGrid* m_grid;
    QSize m_viewSize;
    LoadingBoundaries m_loadingBoundaries;
    float m_verticalTriggerRatio;
    float m_horizontalTriggerRatio;
};

} // namespace MusicTrainer::presentation