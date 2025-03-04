#include "presentation/ViewportManager.h"
#include "presentation/NoteGrid.h"
#include "presentation/ScoreView.h"
#include <QtWidgets/QGraphicsView>
#include <QtMath>
#include <optional>
#include <QDebug>

namespace MusicTrainer::presentation {

ViewportManager::ViewportManager(NoteGrid* grid)
    : m_grid(grid)
    , m_currentState{QRectF(0, 60, 48, 12), 1.0f, QPointF()}  // Initialize with C4-C5 range
    , m_viewSize(0, 0)
    , m_verticalTriggerRatio(0.1f)
    , m_horizontalTriggerRatio(0.2f)
{
    // Set initial dimensions to lock C4-C5 octave
    if (m_grid) {
        auto dimensions = m_grid->getDimensions();
        dimensions.minPitch = 60;  // C4
        dimensions.maxPitch = 72;  // C5
        m_grid->setDimensions(dimensions);
    }
}

ViewportManager::~ViewportManager() = default;

ViewportManager::ViewportState ViewportManager::validateState(const ViewportState& state) const
{
    ViewportState validated = state;
    
    // Ensure exactly one octave height
    validated.visibleArea.setHeight(getOctaveRange());
    
    // Keep viewport within valid MIDI range
    if (validated.visibleArea.top() < 0) {
        validated.visibleArea.moveTop(0);
    } else if (validated.visibleArea.bottom() > 127) {
        validated.visibleArea.moveBottom(127);
    }
    
    return validated;
}

bool ViewportManager::updateViewportState(const ViewportState& newState)
{
    // Get current grid dimensions to maintain octave lock
    auto dimensions = m_grid->getDimensions();
    
    // Convert scroll position and view size to musical coordinates
    QRectF musicalArea(
        newState.scrollPosition.x() / GridConstants::GRID_UNIT,
        dimensions.minPitch,  // Lock to current octave
        m_viewSize.width() / (GridConstants::GRID_UNIT * newState.zoomLevel),
        12  // Force one octave height
    );
    
    // Update state with constrained bounds
    m_currentState = newState;
    m_currentState.visibleArea = musicalArea;
    
    return false;  // No expansion needed since we're locking the octave
}

QRectF ViewportManager::getViewportBounds() const
{
    return m_currentState.visibleArea;
}

void ViewportManager::setViewportBounds(const QRectF& bounds)
{
    if (!m_grid) return;
    
    auto dimensions = m_grid->getDimensions();
    
    // Convert bounds to musical coordinates if needed
    bool isSceneCoords = bounds.width() > 100 || bounds.height() > 100;
    QRectF musicalBounds = isSceneCoords ?
        QRectF(
            bounds.x() / GridConstants::GRID_UNIT,
            dimensions.minPitch,  // Lock to current octave
            bounds.width() / GridConstants::GRID_UNIT,
            12  // Force one octave height
        ) : bounds;
    
    // Maintain current octave and update horizontal bounds only
    dimensions.endPosition = qMin(dimensions.startPosition + 48,
                                qFloor(musicalBounds.right()));
    
    // Update the grid with constrained dimensions
    m_grid->setDimensions(dimensions);
    
    // Update current state with constrained bounds
    m_currentState.visibleArea = musicalBounds;
    m_currentState.visibleArea.setHeight(12);
}

void ViewportManager::updateViewSize(const QSize& size)
{
    m_viewSize = size;
    updateViewportState(m_currentState);
}

void ViewportManager::updateScrollPosition(const QPointF& scrollPos)
{
    m_currentState.scrollPosition = scrollPos;
    updateViewportState(m_currentState);
}

void ViewportManager::updateZoomLevel(float zoom)
{
    m_currentState.zoomLevel = zoom;
    updateViewportState(m_currentState);
}

QPointF ViewportManager::mapToMusicalSpace(const QPointF& screenPoint, const QGraphicsView* view) const
{
    if (!view) return QPointF();
    
    // Convert screen coordinates to scene coordinates
    QPointF scenePoint = view->mapToScene(screenPoint.toPoint());
    
    // Convert scene coordinates to musical coordinates
    scenePoint.rx() /= ScoreView::GRID_UNIT;
    scenePoint.ry() /= ScoreView::NOTE_HEIGHT;
    
    // Apply zoom level
    float inverseZoom = 1.0f / m_currentState.zoomLevel;
    scenePoint *= inverseZoom;
    
    // Add scroll offset to get final musical position
    scenePoint += m_currentState.scrollPosition;
    
    qDebug() << "ViewportManager::mapToMusicalSpace -"
             << "Screen:" << screenPoint
             << "Scene:" << scenePoint
             << "InverseZoom:" << inverseZoom
             << "Final:" << scenePoint;
    
    return scenePoint;
}

QPointF ViewportManager::mapFromMusicalSpace(const QPointF& musicalPoint, const QGraphicsView* view) const
{
    if (!view) return QPointF();
    
    // Remove scroll offset
    QPointF scenePoint = musicalPoint - m_currentState.scrollPosition;
    
    // Apply zoom level
    scenePoint *= m_currentState.zoomLevel;
    
    // Convert to scene coordinates
    scenePoint.rx() *= ScoreView::GRID_UNIT;
    scenePoint.ry() *= ScoreView::NOTE_HEIGHT;
    
    qDebug() << "ViewportManager::mapFromMusicalSpace -"
             << "Musical:" << musicalPoint
             << "WithoutScroll:" << scenePoint
             << "Zoom:" << m_currentState.zoomLevel
             << "Final:" << view->mapFromScene(scenePoint);
    
    // Convert scene coordinates back to screen coordinates
    return view->mapFromScene(scenePoint);
}

void ViewportManager::expandGrid(Direction direction, int amount)
{
    // No vertical expansion allowed - maintain locked octave
    if (direction == Direction::Up || direction == Direction::Down) {
        return;
    }
    
    if (!m_grid) return;
    
    auto dimensions = m_grid->getDimensions();
    
    // Only allow horizontal expansion up to 12 measures
    if (direction == Direction::Right) {
        if (dimensions.endPosition - dimensions.startPosition >= 48) {
            return;
        }
        dimensions.endPosition = qMin(dimensions.startPosition + 48, 
                                   dimensions.endPosition + amount);
        m_grid->setDimensions(dimensions);
    }
}

std::optional<ViewportManager::Direction> ViewportManager::shouldExpand() const 
{
    if (!m_grid) return std::nullopt;
    
    QRectF visibleArea = m_currentState.visibleArea;
    auto dims = m_grid->getDimensions();
    
    // Only trigger octave shifts at near edges
    float edgeThreshold = 0.5; // Half semitone from edge
    
    if (visibleArea.top() <= dims.minPitch + edgeThreshold) {
        return Direction::Up;
    }
    if (visibleArea.bottom() >= dims.maxPitch - edgeThreshold) {
        return Direction::Down;
    }
    
    // Allow horizontal expansion up to 12 measures
    if (visibleArea.right() >= dims.endPosition - 2 && // Within 2 beats of right edge
        dims.endPosition - dims.startPosition < 48) {   // Less than 12 measures
        return Direction::Right;
    }
    
    return std::nullopt;
}

const ViewportManager::ViewportState& ViewportManager::getViewportState() const
{
    return m_currentState;
}

const ViewportManager::LoadingBoundaries& ViewportManager::getLoadingBoundaries() const
{
    return m_loadingBoundaries;
}

void ViewportManager::setLoadingBoundaries(const LoadingBoundaries& boundaries)
{
    m_loadingBoundaries = boundaries;
}

void ViewportManager::compactUnusedRegions()
{
    if (!m_grid) return;
    
    // Calculate buffer zone around visible area
    QRectF bufferZone = m_currentState.visibleArea;
    bufferZone.adjust(
        -m_loadingBoundaries.horizontalBuffer,
        -m_loadingBoundaries.verticalBuffer,
        m_loadingBoundaries.horizontalBuffer,
        m_loadingBoundaries.verticalBuffer
    );
    
    m_grid->compactUnusedRegions(m_currentState.visibleArea, bufferZone);
}

} // namespace MusicTrainer::presentation