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
    , m_currentState{QRectF(), 1.0f, QPointF()}
    , m_viewSize(0, 0)
    , m_verticalTriggerRatio(0.1f)
    , m_horizontalTriggerRatio(0.2f)
{
}

ViewportManager::~ViewportManager() = default;

bool ViewportManager::updateViewportState(const ViewportState& newState)
{
    // Convert scroll position and view size to musical coordinates
    QRectF musicalArea(
        newState.scrollPosition.x() / GridConstants::GRID_UNIT,          // Convert x to beats
        newState.scrollPosition.y() / GridConstants::NOTE_HEIGHT,        // Convert y to semitones
        m_viewSize.width() / (GridConstants::GRID_UNIT * newState.zoomLevel),     // Width in beats
        m_viewSize.height() / (GridConstants::NOTE_HEIGHT * newState.zoomLevel)    // Height in semitones
    );
    
    qDebug() << "ViewportManager::updateViewportState -"
             << "MusicalArea:" << musicalArea
             << "Zoom:" << newState.zoomLevel
             << "ScrollPos:" << newState.scrollPosition;
    
    // Update state
    m_currentState = newState;
    m_currentState.visibleArea = musicalArea;
    
    // Check if we need to expand based on the current viewport bounds
    auto direction = shouldExpand();
    if (direction) {
        // Calculate expansion amount based on direction
        int amount = (*direction == Direction::Right) ? 16 : 12;
        qDebug() << "ViewportManager::expandGrid -" 
                 << "Direction:" << static_cast<int>(*direction)
                 << "Amount:" << amount;
        expandGrid(*direction, amount);
        return true;
    }
    return false;
}

QRectF ViewportManager::getViewportBounds() const
{
    return m_currentState.visibleArea;
}

void ViewportManager::setViewportBounds(const QRectF& bounds)
{
    // If no grid is set, we can't process bounds
    if (!m_grid) return;
    
    // Get current grid dimensions before any changes
    auto dimensions = m_grid->getDimensions();
    
    // Convert bounds to musical coordinates if needed (handle both scene and musical coords)
    bool isSceneCoords = bounds.width() > 100 || bounds.height() > 100;
    
    QRectF musicalBounds = isSceneCoords ?
        QRectF(
            bounds.x() / GridConstants::GRID_UNIT,
            bounds.y() / GridConstants::NOTE_HEIGHT,
            bounds.width() / GridConstants::GRID_UNIT,
            bounds.height() / GridConstants::NOTE_HEIGHT
        ) : bounds;
    
    // Maintain one octave constraint (12 semitones)
    int currentRange = dimensions.maxPitch - dimensions.minPitch;
    if (currentRange != 12) {
        // Reset to one octave if it's different
        dimensions.maxPitch = dimensions.minPitch + 12;
    }
    
    // Maintain 12 measure constraint (48 beats in 4/4 time)
    if (dimensions.endPosition - dimensions.startPosition != 48) {
        dimensions.endPosition = dimensions.startPosition + 48;
    }
    
    // Update the grid with constrained dimensions
    m_grid->setDimensions(dimensions);
    
    // Update current state with constrained bounds
    m_currentState.visibleArea = musicalBounds;
    m_currentState.visibleArea.setHeight(12); // Force one octave height
    
    // If we're at the edge, shift the viewport to keep within bounds
    if (m_currentState.visibleArea.bottom() > dimensions.maxPitch) {
        m_currentState.visibleArea.moveBottom(dimensions.maxPitch);
    }
    if (m_currentState.visibleArea.top() < dimensions.minPitch) {
        m_currentState.visibleArea.moveTop(dimensions.minPitch);
    }
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
    // Expansion is now constrained - it only shifts the visible octave up/down
    // rather than actually expanding the grid
    if (!m_grid) return;
    
    auto dimensions = m_grid->getDimensions();
    
    switch (direction) {
        case Direction::Up:
            // Shift the octave up
            dimensions.minPitch -= 12;
            dimensions.maxPitch -= 12;
            break;
            
        case Direction::Down:
            // Shift the octave down
            dimensions.minPitch += 12;
            dimensions.maxPitch += 12;
            break;
            
        case Direction::Right:
            // No horizontal expansion beyond 12 measures
            if (dimensions.endPosition - dimensions.startPosition >= 48) {
                return;
            }
            dimensions.endPosition = qMin(dimensions.endPosition + amount, 
                                        dimensions.startPosition + 48);
            break;
    }
    
    m_grid->setDimensions(dimensions);
}

std::optional<ViewportManager::Direction> ViewportManager::shouldExpand() const
{
    if (!m_grid) return std::nullopt;
    
    QRectF visibleArea = m_currentState.visibleArea;
    auto dims = m_grid->getDimensions();
    
    // Convert grid dimensions to musical coordinates for comparison
    float gridWidth = (dims.endPosition - dims.startPosition);
    float gridHeight = (dims.maxPitch - dims.minPitch);
    
    // Check vertical expansion (pitch range)
    if (visibleArea.top() <= dims.minPitch + gridHeight * m_verticalTriggerRatio) {
        return Direction::Up;
    }
    if (visibleArea.bottom() >= dims.maxPitch - gridHeight * m_verticalTriggerRatio) {
        return Direction::Down;
    }
    
    // Check horizontal expansion (time)
    if (visibleArea.right() >= dims.endPosition - gridWidth * m_horizontalTriggerRatio) {
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