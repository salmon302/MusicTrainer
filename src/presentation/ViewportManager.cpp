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
    if (!m_grid) return state;
    
    ViewportState validated = state;

    // Convert current dimensions to musical space
    auto dimensions = m_grid->getDimensions();
    
    // Always maintain exactly one octave height
    validated.visibleArea.setHeight(getOctaveRange());
    
    // Clamp pitch range to valid MIDI range
    if (validated.visibleArea.top() < 0) {
        validated.visibleArea.moveTop(0);
    } else if (validated.visibleArea.bottom() > 127) {
        validated.visibleArea.moveBottom(127);
    }
    
    // Ensure scroll position matches validated bounds
    validated.scrollPosition = QPointF(
        validated.visibleArea.x() * GridConstants::GRID_UNIT,
        qBound(0.0, validated.visibleArea.y(), 115.0) * GridConstants::NOTE_HEIGHT
    );

    // Ensure zoom level is reasonable
    validated.zoomLevel = qBound(0.1f, validated.zoomLevel, 10.0f);
    
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
        newState.preserveOctaveExpansion ? dimensions.minPitch : dimensions.minPitch,  // Respect expansion state
        m_viewSize.width() / (GridConstants::GRID_UNIT * newState.zoomLevel),
        newState.preserveOctaveExpansion ? dimensions.maxPitch - dimensions.minPitch : 12  // Preserve expanded height if needed
    );
    
    // Update state with bounded coordinates
    m_currentState = newState;
    m_currentState.visibleArea = musicalArea;
    
    // Preserve multi-octave display if it was previously set
    if (!m_currentState.preserveOctaveExpansion) {
        m_currentState.visibleArea.setHeight(12);  // Default to one octave height
    }
    
    // Only update grid dimensions if they would actually change
    // and we're not in multi-octave mode
    if (!m_currentState.preserveOctaveExpansion &&
        (dimensions.minPitch != qFloor(musicalArea.top()) ||
         dimensions.maxPitch != qFloor(musicalArea.top()) + 12)) {
        dimensions.minPitch = qBound(0, qFloor(musicalArea.top()), 115);
        dimensions.maxPitch = dimensions.minPitch + 12;  // Maintain octave
        m_grid->setDimensions(dimensions);
    }
    
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
    
    // Calculate new dimensions based on the viewport bounds
    auto newDimensions = dimensions;
    
    // Only update pitch range if we're not in multi-octave mode
    if (!m_currentState.preserveOctaveExpansion) {
        newDimensions.minPitch = qFloor(bounds.top());
        newDimensions.maxPitch = newDimensions.minPitch + getOctaveRange();
    }
    
    // For horizontal expansion, we want to extend if we're near the right edge
    if (bounds.right() > dimensions.endPosition - 2) { // Within 2 beats of the edge
        int newEndPosition = qMin(48, qCeil(bounds.right() + 4)); // Add some buffer but cap at 48
        newDimensions.endPosition = newEndPosition;
    }
    
    // Update grid dimensions if they've changed and we're not in multi-octave mode
    if (!m_currentState.preserveOctaveExpansion && 
        (newDimensions.minPitch != dimensions.minPitch ||
         newDimensions.maxPitch != dimensions.maxPitch ||
         newDimensions.endPosition != dimensions.endPosition)) {
        m_grid->setDimensions(newDimensions);
        dimensions = newDimensions; // Update local copy
    }
    
    // Update viewport state, preserving height in multi-octave mode
    m_currentState.visibleArea = bounds;
    if (!m_currentState.preserveOctaveExpansion) {
        m_currentState.visibleArea.setHeight(12);  // Force one octave height only in single-octave mode
    }
    
    // Convert to scene coordinates and update scroll position, respecting multi-octave mode
    m_currentState.scrollPosition = QPointF(
        bounds.x() * GridConstants::GRID_UNIT,
        (m_currentState.preserveOctaveExpansion ? bounds.y() : dimensions.minPitch) * GridConstants::NOTE_HEIGHT
    );
    
    qDebug() << "ViewportManager::setViewportBounds -"
             << "Bounds:" << bounds
             << "MultiOctave:" << m_currentState.preserveOctaveExpansion
             << "MinPitch:" << dimensions.minPitch
             << "MaxPitch:" << dimensions.maxPitch;
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
    
    // Skip update if movement is negligible
    if (qAbs(scrollPos.x() - oldPos.x()) < 1.0 && qAbs(scrollPos.y() - oldPos.y()) < 1.0) {
        return;
    }
    
    // Update with new position
    m_currentState.scrollPosition = scrollPos;
    
    // Calculate scroll delta in musical coordinates
    QPointF scrollDelta(
        (scrollPos.x() - oldPos.x()) / GridConstants::GRID_UNIT,
        (scrollPos.y() - oldPos.y()) / GridConstants::NOTE_HEIGHT  // Allow vertical scrolling in multi-octave mode
    );
    
    // Update visible area based on scroll delta
    if (m_currentState.preserveOctaveExpansion) {
        // In multi-octave mode, allow both horizontal and vertical scrolling
        m_currentState.visibleArea.translate(scrollDelta.x(), scrollDelta.y());
    } else {
        // In single-octave mode, only allow horizontal scrolling
        m_currentState.visibleArea.translate(scrollDelta.x(), 0);
    }
    
    // Get current grid dimensions to maintain proper bounds
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
    
    // Handle vertical bounds differently based on mode
    if (m_currentState.preserveOctaveExpansion) {
        // In multi-octave mode, clamp to the expanded range
        if (m_currentState.visibleArea.top() < dimensions.minPitch) {
            m_currentState.visibleArea.moveTop(dimensions.minPitch);
            m_currentState.scrollPosition.setY(dimensions.minPitch * GridConstants::NOTE_HEIGHT);
        }
        if (m_currentState.visibleArea.bottom() > dimensions.maxPitch) {
            m_currentState.visibleArea.moveBottom(dimensions.maxPitch);
            m_currentState.scrollPosition.setY((dimensions.maxPitch - m_currentState.visibleArea.height()) 
                * GridConstants::NOTE_HEIGHT);
        }
    } else {
        // In single-octave mode, lock to current octave
        m_currentState.visibleArea.moveTop(dimensions.minPitch);
        m_currentState.scrollPosition.setY(dimensions.minPitch * GridConstants::NOTE_HEIGHT);
    }
    
    qDebug() << "ViewportManager::updateScrollPosition -"
             << "MultiOctave:" << m_currentState.preserveOctaveExpansion
             << "ScrollPos:" << scrollPos
             << "VisibleArea:" << m_currentState.visibleArea;
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
    int oldMaxPitch = dimensions.maxPitch;
    
    // Store view center before expansion
    int viewCenterX = qRound(m_currentState.visibleArea.center().x());
    
    // Flag to track if multiple octaves are being displayed
    bool multiOctaveMode = false;
    
    switch (direction) {
        case Direction::Up:
            if (dimensions.minPitch >= 12) {  // Ensure room for full octave above
                // Instead of shifting, expand to include another octave above
                dimensions.minPitch -= 12;
                multiOctaveMode = true; // Mark that we're in multi-octave mode
                // Keep the max pitch the same to show both octaves
            }
            break;
            
        case Direction::Down:
            if (dimensions.maxPitch <= 115) {  // Ensure room for full octave below
                // Instead of shifting, expand to include another octave below
                dimensions.maxPitch += 12;
                multiOctaveMode = true; // Mark that we're in multi-octave mode
                // Keep the min pitch the same to show both octaves
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
    
    // Calculate new viewport state
    float visibleWidth = m_currentState.visibleArea.width();
    int visibleOctaves = (dimensions.maxPitch - dimensions.minPitch) / 12;
    
    // Update visible area to show the expanded octave range
    m_currentState.visibleArea = QRectF(
        viewCenterX - visibleWidth / 2,                // Keep horizontal center
        dimensions.minPitch,                          // Start at new min pitch
        visibleWidth,                                 // Maintain visible width
        dimensions.maxPitch - dimensions.minPitch     // Show full expanded range
    );
    
    // Update scroll position to match new dimensions and viewport
    m_currentState.scrollPosition = QPointF(
        m_currentState.visibleArea.x() * GridConstants::GRID_UNIT,
        dimensions.minPitch * GridConstants::NOTE_HEIGHT
    );
    
    // Store the multi-octave state as part of the viewport state
    if (multiOctaveMode) {
        // Force the viewport to maintain the expanded octave range
        m_currentState.preserveOctaveExpansion = true;
    }
    
    // Force update
    updateViewportState(m_currentState);
    
    // Get scene for visual updates
    QGraphicsScene* scene = m_grid->getScene();
    if (scene) {
        // Ensure scene rect is updated to match the new dimensions
        float labelMargin = 25.0f;
        QRectF newSceneRect(
            -labelMargin,  // Space for labels
            dimensions.minPitch * GridConstants::NOTE_HEIGHT - GridConstants::NOTE_HEIGHT * 0.5,  // Small padding above
            dimensions.endPosition * GridConstants::GRID_UNIT + 2 * labelMargin,  // Width plus margins
            (dimensions.maxPitch - dimensions.minPitch + 2) * GridConstants::NOTE_HEIGHT  // Full expanded range plus space for buttons
        );
        scene->setSceneRect(newSceneRect);
        
        // Signal that update is complete
        if (!scene->views().empty()) {
            auto view = scene->views().first();
            view->viewport()->update();
            
            // Force a full scene update
            scene->update(scene->sceneRect());
        }
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