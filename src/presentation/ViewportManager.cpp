#include "presentation/ViewportManager.h"
#include "presentation/NoteGrid.h"
#include "presentation/ScoreView.h"
#include "presentation/GridConstants.h"
#include <QtWidgets/QGraphicsView>
#include <QtMath>
#include <optional>
#include <QDebug>

namespace MusicTrainer::presentation {

using namespace GridConstants;

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
    auto dimensions = m_grid->getDimensions();
    
    // Handle height based on multi-octave mode
    if (!validated.preserveOctaveExpansion) {
        // In single-octave mode, enforce exactly one octave height
        validated.visibleArea.setHeight(getOctaveRange());
    } else {
        // In multi-octave mode, preserve the expanded height but ensure it's a multiple of octaves
        int currentHeight = qRound(validated.visibleArea.height());
        if (currentHeight % getOctaveRange() != 0) {
            int octaves = (currentHeight / getOctaveRange()) + 1;
            validated.visibleArea.setHeight(octaves * getOctaveRange());
        }
    }
    
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
        newState.visibleArea == m_currentState.visibleArea &&
        newState.preserveOctaveExpansion == m_currentState.preserveOctaveExpansion) {
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
    
    // Only update grid dimensions if explicitly requested via setViewportBounds
    // This prevents note placement from affecting the viewport
    return false;  // Don't trigger expansion during state updates
}

QRectF ViewportManager::getViewportBounds() const
{
    return m_currentState.visibleArea;
}

void ViewportManager::setViewportBounds(const QRectF& bounds)
{
    if (!m_grid) return;
    
    auto dimensions = m_grid->getDimensions();
    auto newDimensions = dimensions;
    
    // Only update pitch range if we're not in multi-octave mode
    if (!m_currentState.preserveOctaveExpansion) {
        newDimensions.minPitch = qFloor(bounds.top());
        newDimensions.maxPitch = newDimensions.minPitch + getOctaveRange();
    }
    
    // For horizontal expansion, extend if we're near the right edge
    if (bounds.right() > dimensions.endPosition - 2) { 
        int newEndPosition = qMin(192, qCeil(bounds.right() + 4)); // Add buffer but cap at 48 measures
        newDimensions.endPosition = newEndPosition;
    }
    
    // Validate and apply new dimensions
    m_grid->setDimensions(newDimensions);
    
    // Update viewport state to match new bounds
    m_currentState.visibleArea = bounds;
    updateViewportState(m_currentState);
    
    // Update viewport visually
    if (m_grid->getScene()) {
        m_grid->getScene()->update();
    }
}

void ViewportManager::updateViewSize(const QSize& size)
{
    if (m_viewSize == size) return;
    
    m_viewSize = size;
    auto dimensions = m_grid->getDimensions();
    
    // Calculate actual content size in scene units
    float labelMargin = 25.0f;
    float topMargin = 30.0f;
    float bottomMargin = 15.0f;
    
    float contentWidth = dimensions.endPosition * GridConstants::GRID_UNIT + 2 * labelMargin;
    float contentHeight = (dimensions.maxPitch - dimensions.minPitch) * GridConstants::NOTE_HEIGHT + 
                         topMargin + bottomMargin;
    
    // Calculate optimal scale to fit content
    float horizontalScale = size.width() / contentWidth;
    float verticalScale = size.height() / contentHeight;
    
    // Use minimum scale to ensure all content fits
    float newZoom = qMin(horizontalScale, verticalScale);
    
    // Clamp zoom to reasonable range
    newZoom = qMax(0.8f, qMin(newZoom, 1.5f));
    
    // Update state
    m_currentState.zoomLevel = newZoom;
    updateViewportState(m_currentState);
}

void ViewportManager::updateScrollPosition(const QPointF& scrollPos)
{
    // Skip if movement is negligible
    if (qAbs(scrollPos.x() - m_currentState.scrollPosition.x()) < 1.0 &&
        qAbs(scrollPos.y() - m_currentState.scrollPosition.y()) < 1.0) {
        return;
    }
    
    // Store old position for delta calculation
    QPointF oldScroll = m_currentState.scrollPosition;
    
    // Update scroll position
    m_currentState.scrollPosition = scrollPos;
    
    // Calculate scroll delta in musical coordinates
    QPointF scrollDelta = calculateScaledDelta(scrollPos, oldScroll);
    
    // Update visible area
    if (m_currentState.preserveOctaveExpansion) {
        // In multi-octave mode, allow full scrolling
        m_currentState.visibleArea.translate(scrollDelta.x(), scrollDelta.y());
    } else {
        // In single-octave mode, only allow horizontal scrolling
        m_currentState.visibleArea.translate(scrollDelta.x(), 0);
    }
    
    // Get current dimensions
    auto dimensions = m_grid->getDimensions();
    
    // Enforce bounds
    if (m_currentState.visibleArea.left() < 0) {
        m_currentState.visibleArea.moveLeft(0);
        m_currentState.scrollPosition.setX(0);
    }
    if (m_currentState.visibleArea.right() > dimensions.endPosition) {
        m_currentState.visibleArea.moveRight(dimensions.endPosition);
        m_currentState.scrollPosition.setX((dimensions.endPosition - m_currentState.visibleArea.width()) 
            * GridConstants::GRID_UNIT * m_currentState.zoomLevel);
    }
    
    // Handle vertical bounds based on mode
    if (m_currentState.preserveOctaveExpansion) {
        if (m_currentState.visibleArea.top() < dimensions.minPitch) {
            m_currentState.visibleArea.moveTop(dimensions.minPitch);
            m_currentState.scrollPosition.setY(dimensions.minPitch * GridConstants::NOTE_HEIGHT 
                * m_currentState.zoomLevel);
        }
        if (m_currentState.visibleArea.bottom() > dimensions.maxPitch) {
            m_currentState.visibleArea.moveBottom(dimensions.maxPitch);
            m_currentState.scrollPosition.setY((dimensions.maxPitch - m_currentState.visibleArea.height()) 
                * GridConstants::NOTE_HEIGHT * m_currentState.zoomLevel);
        }
    } else {
        // Lock to current octave in single-octave mode
        m_currentState.visibleArea.moveTop(dimensions.minPitch);
        m_currentState.scrollPosition.setY(dimensions.minPitch * GridConstants::NOTE_HEIGHT 
            * m_currentState.zoomLevel);
    }
    
    // Update state
    updateViewportState(m_currentState);
}

void ViewportManager::updateZoomLevel(float zoom)
{
    m_currentState.zoomLevel = zoom;
    updateViewportState(m_currentState);
}

QPointF ViewportManager::mapToMusicalSpace(const QPointF& screenPoint, const QGraphicsView* view) const
{
    if (!view || !m_grid) return QPointF();
    
    // Convert screen point to scene coordinates
    QPointF scenePoint = view->mapToScene(screenPoint.toPoint());
    
    // Account for current zoom level and grid unit scaling
    double musicalX = scenePoint.x() / (GridConstants::GRID_UNIT * m_currentState.zoomLevel);
    double musicalY = scenePoint.y() / (GridConstants::NOTE_HEIGHT * m_currentState.zoomLevel);
    
    // Add scroll offset
    musicalX += m_currentState.scrollPosition.x() / (GridConstants::GRID_UNIT * m_currentState.zoomLevel);
    musicalY += m_currentState.scrollPosition.y() / (GridConstants::NOTE_HEIGHT * m_currentState.zoomLevel);
    
    return QPointF(musicalX, musicalY);
}

QPointF ViewportManager::mapFromMusicalSpace(const QPointF& musicalPoint, const QGraphicsView* view) const
{
    if (!view || !m_grid) return QPointF();
    
    // First convert to scene coordinates with proper scaling
    QPointF scenePoint(
        musicalPoint.x() * GridConstants::GRID_UNIT * m_currentState.zoomLevel,
        musicalPoint.y() * GridConstants::NOTE_HEIGHT * m_currentState.zoomLevel
    );
    
    // Remove scroll offset
    scenePoint -= m_currentState.scrollPosition;
    
    // Convert back to screen coordinates
    return view->mapFromScene(scenePoint);
}

void ViewportManager::expandGrid(GridDirection direction, int amount)
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
        case GridDirection::Up:
            if (dimensions.minPitch >= 12) {  // Ensure room for full octave above
                dimensions.minPitch -= 12;
                multiOctaveMode = true;
            }
            break;
            
        case GridDirection::Down:
            if (dimensions.maxPitch <= 115) {  // Ensure room for full octave below
                dimensions.maxPitch += 12;
                multiOctaveMode = true;
            }
            break;
            
        case GridDirection::Right:
            if (dimensions.endPosition - dimensions.startPosition < 192) {
                int expansion = qMin(amount, 192 - (dimensions.endPosition - dimensions.startPosition));
                dimensions.endPosition += expansion;
            }
            break;
    }
    
    // Apply new dimensions first
    m_grid->setDimensions(dimensions);
    
    // Calculate new viewport state
    float visibleWidth = m_currentState.visibleArea.width();
    int visibleOctaves = (dimensions.maxPitch - dimensions.minPitch) / 12;
    
    m_currentState.visibleArea = QRectF(
        viewCenterX - visibleWidth / 2,
        dimensions.minPitch,
        visibleWidth,
        dimensions.maxPitch - dimensions.minPitch
    );
    
    m_currentState.scrollPosition = QPointF(
        m_currentState.visibleArea.x() * GridConstants::GRID_UNIT,
        dimensions.minPitch * GridConstants::NOTE_HEIGHT
    );
    
    if (multiOctaveMode) {
        m_currentState.preserveOctaveExpansion = true;
    }
    
    updateViewportState(m_currentState);
    
    QGraphicsScene* scene = m_grid->getScene();
    if (scene) {
        float labelMargin = 25.0f;
        QRectF newSceneRect(
            -labelMargin,
            dimensions.minPitch * GridConstants::NOTE_HEIGHT - GridConstants::NOTE_HEIGHT * 0.5,
            dimensions.endPosition * GridConstants::GRID_UNIT + 2 * labelMargin,
            (dimensions.maxPitch - dimensions.minPitch + 2) * GridConstants::NOTE_HEIGHT
        );
        scene->setSceneRect(newSceneRect);
        
        if (!scene->views().empty()) {
            auto view = scene->views().first();
            view->viewport()->update();
            scene->update(scene->sceneRect());
        }
    }
}

void ViewportManager::collapseGrid(GridDirection direction)
{
    if (!m_grid) return;
    
    auto dimensions = m_grid->getDimensions();
    int viewCenterX = qRound(m_currentState.visibleArea.center().x());
    bool needsUpdate = false;
    
    NoteGrid::GridDimensions newDimensions = dimensions;
    
    switch (direction) {
        case GridDirection::Up:
            if (dimensions.maxPitch - dimensions.minPitch > 12) {
                newDimensions.minPitch += 12;
                needsUpdate = true;
            }
            break;
            
        case GridDirection::Down:
            if (dimensions.maxPitch - dimensions.minPitch > 12) {
                newDimensions.maxPitch -= 12;
                needsUpdate = true;
            }
            break;
            
        case GridDirection::Right:
            if (dimensions.endPosition > 16) {
                int newEndPosition = qMax(16, dimensions.endPosition - 4);
                int notesRemoved = m_grid->removeNotesInRange(newEndPosition, dimensions.endPosition);
                newDimensions.endPosition = newEndPosition;
                needsUpdate = true;
            }
            break;
    }
    
    if (needsUpdate) {
        m_grid->setDimensions(newDimensions);
        
        m_currentState.preserveOctaveExpansion = (newDimensions.maxPitch - newDimensions.minPitch > 12);
        m_currentState.visibleArea = QRectF(
            viewCenterX - m_currentState.visibleArea.width() / 2,
            newDimensions.minPitch,
            m_currentState.visibleArea.width(),
            newDimensions.maxPitch - newDimensions.minPitch
        );
        
        m_currentState.scrollPosition = QPointF(
            m_currentState.visibleArea.x() * GridConstants::GRID_UNIT,
            newDimensions.minPitch * GridConstants::NOTE_HEIGHT
        );
    }
}

bool ViewportManager::canCollapse(GridDirection direction) const
{
    if (!m_grid) return false;
    
    auto dimensions = m_grid->getDimensions();
    
    switch (direction) {
        case GridDirection::Up:
        case GridDirection::Down:
            return dimensions.maxPitch - dimensions.minPitch > 12;
            
        case GridDirection::Right:
            return dimensions.endPosition > 16;
    }
    
    return false;
}

std::optional<GridDirection> ViewportManager::shouldExpand() const 
{
    if (!m_grid) return std::nullopt;
    
    QRectF visibleArea = m_currentState.visibleArea;
    auto dims = m_grid->getDimensions();
    
    float edgeThreshold = 0.5;
    
    if (visibleArea.top() <= dims.minPitch + edgeThreshold) {
        return GridDirection::Up;
    }
    if (visibleArea.bottom() >= dims.maxPitch - edgeThreshold) {
        return GridDirection::Down;
    }
    
    if (visibleArea.right() >= dims.endPosition - 2 && 
        dims.endPosition - dims.startPosition < 192) {
        return GridDirection::Right;
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

QPointF ViewportManager::calculateScaledDelta(const QPointF& screen, const QPointF& scene) const
{
    // Convert both points to musical coordinates for consistent scaling
    double screenX = screen.x() / (GridConstants::GRID_UNIT * m_currentState.zoomLevel);
    double screenY = screen.y() / (GridConstants::NOTE_HEIGHT * m_currentState.zoomLevel);
    double sceneX = scene.x() / GridConstants::GRID_UNIT;
    double sceneY = scene.y() / GridConstants::NOTE_HEIGHT;
    
    // Return the difference in musical coordinates
    return QPointF(screenX - sceneX, screenY - sceneY);
}

} // namespace MusicTrainer::presentation