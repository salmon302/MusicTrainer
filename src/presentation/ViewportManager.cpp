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
    m_currentState = newState;
    
    // Calculate scaled area for debugging output only - do not modify m_currentState
    QRectF scaledArea = m_currentState.visibleArea;
    scaledArea.setLeft(scaledArea.left() * ScoreView::GRID_UNIT);
    scaledArea.setRight(scaledArea.right() * ScoreView::GRID_UNIT);
    scaledArea.setTop(scaledArea.top() * ScoreView::NOTE_HEIGHT);
    scaledArea.setBottom(scaledArea.bottom() * ScoreView::NOTE_HEIGHT);
    
    qDebug() << "ViewportManager::updateViewportState -"
             << "Original:" << newState.visibleArea
             << "Scaled:" << scaledArea
             << "Zoom:" << m_currentState.zoomLevel
             << "ScrollPos:" << m_currentState.scrollPosition;
    
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
    // Store original, unscaled bounds
    m_currentState.visibleArea = bounds;
    updateViewportState(m_currentState);
}

void ViewportManager::updateViewSize(const QSize& size)
{
    m_viewSize = size;
    // Recalculate viewport state based on new size
    updateViewportState(m_currentState);
}

void ViewportManager::updateScrollPosition(const QPointF& pos)
{
    m_currentState.scrollPosition = pos;
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
    if (!m_grid) return;
    
    auto dims = m_grid->getDimensions();
    switch (direction) {
        case Direction::Up:
            m_grid->expandVertical(-amount, 0);
            break;
        case Direction::Down:
            m_grid->expandVertical(0, amount);
            break;
        case Direction::Right:
            m_grid->expandHorizontal(amount);
            break;
    }
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