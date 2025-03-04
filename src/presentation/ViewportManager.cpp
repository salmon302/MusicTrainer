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
    // Only update if there's a meaningful change
    if (qFuzzyCompare(newState.scrollPosition.x(), m_currentState.scrollPosition.x()) &&
        qFuzzyCompare(newState.scrollPosition.y(), m_currentState.scrollPosition.y()) &&
        qFuzzyCompare(newState.zoomLevel, m_currentState.zoomLevel) &&
        newState.visibleArea == m_currentState.visibleArea) {
        return false;
    }

    // Get current grid dimensions
    auto dimensions = m_grid->getDimensions();
    
    // Convert scroll position and view size to musical coordinates
    QRectF musicalArea(
        qMax(0.0, newState.scrollPosition.x() / GridConstants::GRID_UNIT),
        dimensions.minPitch,  // Lock to current octave
        m_viewSize.width() / (GridConstants::GRID_UNIT * newState.zoomLevel),
        12  // Force one octave height
    );
    
    // Update state with bounded coordinates
    m_currentState = newState;
    m_currentState.visibleArea = musicalArea;
    m_currentState.visibleArea.setHeight(12);  // Ensure octave height
    
    // Update the grid
    m_grid->setDimensions(dimensions);
    
    return shouldExpand().has_value();  // Return true if expansion is needed
}

QRectF ViewportManager::getViewportBounds() const
{
    return m_currentState.visibleArea;
}

void ViewportManager::setViewportBounds(const QRectF& bounds)
{
    if (!m_grid) return;
    
    auto dimensions = m_grid->getDimensions();
    
    // Convert musical space coordinates to scene coordinates
    QRectF sceneCoords = QRectF(
        bounds.x() * GridConstants::GRID_UNIT,
        bounds.y() * GridConstants::NOTE_HEIGHT,
        bounds.width() * GridConstants::GRID_UNIT,
        bounds.height() * GridConstants::NOTE_HEIGHT
    );
    
    // Update visible area in musical coordinates
    m_currentState.visibleArea = bounds;
    m_currentState.visibleArea.setHeight(12);  // Force one octave height
    
    // Update scroll position to match new bounds
    m_currentState.scrollPosition = QPointF(
        sceneCoords.x(),
        dimensions.minPitch * GridConstants::NOTE_HEIGHT
    );
    
    // Update the grid
    m_grid->setDimensions(dimensions);
}

void ViewportManager::updateViewSize(const QSize& size)
{
    m_viewSize = size;
    updateViewportState(m_currentState);
}

void ViewportManager::updateScrollPosition(const QPointF& scrollPos)
{
    // Store the old position for delta calculation
    QPointF oldPos = m_currentState.scrollPosition;
    
    // Update with new position
    m_currentState.scrollPosition = scrollPos;
    
    // Calculate scroll delta in musical coordinates
    QPointF scrollDelta(
        (scrollPos.x() - oldPos.x()) / GridConstants::GRID_UNIT,
        (scrollPos.y() - oldPos.y()) / GridConstants::NOTE_HEIGHT
    );
    
    // Update visible area based on scroll delta
    m_currentState.visibleArea.translate(scrollDelta.x(), 0);  // Only allow horizontal scrolling
    
    // Get current grid dimensions to maintain octave lock
    auto dimensions = m_grid->getDimensions();
    
    // Ensure visible area stays within grid bounds
    if (m_currentState.visibleArea.left() < 0) {
        m_currentState.visibleArea.moveLeft(0);
        m_currentState.scrollPosition.setX(0);
    }
    if (m_currentState.visibleArea.right() > dimensions.endPosition) {
        m_currentState.visibleArea.moveRight(dimensions.endPosition);
        m_currentState.scrollPosition.setX((dimensions.endPosition - m_currentState.visibleArea.width()) 
            * GridConstants::GRID_UNIT);
    }
    
    // Lock vertical position to current octave
    m_currentState.visibleArea.moveTop(dimensions.minPitch);
    m_currentState.scrollPosition.setY(dimensions.minPitch * GridConstants::NOTE_HEIGHT);
    
    // Update the viewport state
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
    
    // Get current grid dimensions for proper mapping
    auto dimensions = m_grid->getDimensions();
    
    // Convert scene coordinates to musical coordinates
    double musicalX = scenePoint.x() / ScoreView::GRID_UNIT;
    
    // Fix: Map Y coordinate directly to pitch value based on note height
    double musicalY = scenePoint.y() / ScoreView::NOTE_HEIGHT;
    
    qDebug() << "ViewportManager::mapToMusicalSpace -"
             << "Screen:" << screenPoint
             << "Scene:" << scenePoint
             << "Musical coordinates:" << QPointF(musicalX, musicalY)
             << "Zoom:" << m_currentState.zoomLevel;
    
    return QPointF(musicalX, musicalY);
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
    
    auto dimensions = m_grid->getDimensions();
    int oldMinPitch = dimensions.minPitch;
    
    // Store view center before expansion
    int viewCenterX = qRound(m_currentState.visibleArea.center().x());
    int viewCenterY = dimensions.minPitch + 6;  // Center of octave
    
    switch (direction) {
        case Direction::Up:
            if (dimensions.minPitch >= 12) {
                dimensions.minPitch -= 12;
                dimensions.maxPitch -= 12;
            }
            break;
            
        case Direction::Down:
            if (dimensions.maxPitch <= 115) {
                dimensions.minPitch += 12;
                dimensions.maxPitch += 12;
            }
            break;
            
        case Direction::Right:
            if (dimensions.endPosition - dimensions.startPosition < 48) {
                int expansion = qMin(amount, 48 - (dimensions.endPosition - dimensions.startPosition));
                dimensions.endPosition += expansion;
            }
            break;
    }
    
    // Apply new dimensions first
    m_grid->setDimensions(dimensions);
    
    // Update viewport state to match new dimensions
    m_currentState.visibleArea = QRectF(
        viewCenterX - m_currentState.visibleArea.width() / 2,
        dimensions.minPitch,
        m_currentState.visibleArea.width(),
        12  // One octave
    );
    
    // Update scroll position to match new dimensions
    m_currentState.scrollPosition = QPointF(
        m_currentState.visibleArea.x() * GridConstants::GRID_UNIT,
        dimensions.minPitch * GridConstants::NOTE_HEIGHT
    );
    
    // Notify state change
    updateViewportState(m_currentState);
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