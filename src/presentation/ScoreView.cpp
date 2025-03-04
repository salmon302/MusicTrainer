#include "presentation/ScoreView.h"
#include "presentation/ViewportManager.h"
#include "presentation/NoteGrid.h"
#include "presentation/QtTypeRegistration.h"
#include "domain/music/Note.h"
#include "domain/music/Score.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include <QScrollBar>
#include <QResizeEvent>
#include <QGraphicsRectItem>
#include <QWheelEvent>
#include <QTimer>
#include <QDebug>

namespace MusicTrainer::presentation {

// Define base zoom level constant
constexpr float GRID_ZOOM_BASE = 1.0f;

// Define ScoreViewImpl class to fix incomplete type error
class ScoreView::ScoreViewImpl {
public:
    explicit ScoreViewImpl() = default;
};

// Main implementation
ScoreView::ScoreView(QWidget *parent)
    : QGraphicsView(parent)
    , m_impl(std::make_unique<ScoreViewImpl>())
{
    // Register Qt types for signal/slot system
    ::registerQtTypes();  // Call the global function

    setScene(new QGraphicsScene(this));
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    
    // Initialize with base transform - log initial state
    QTransform baseTransform = QTransform::fromScale(1.0, 1.0);
    qDebug() << "ScoreView::Constructor - Initial transform:" << baseTransform
             << "m11:" << baseTransform.m11()
             << "m22:" << baseTransform.m22();
    setTransform(baseTransform);
    
    m_noteGrid = std::make_unique<NoteGrid>(scene());
    m_viewportManager = std::make_unique<ViewportManager>(m_noteGrid.get());
    
    // Set up scene with minimal initial rect until initialization
    scene()->setSceneRect(0, 0, 100, 100);
    qDebug() << "ScoreView::Constructor - Scene rect:" << scene()->sceneRect();
    
    // Connect the noteAdded signal to our handling slot
    connect(this, &ScoreView::noteAdded, this, &ScoreView::handleNoteAdded);
    
    initializeViewport();
}

ScoreView::~ScoreView() = default;

void ScoreView::setScore(std::shared_ptr<MusicTrainer::music::Score> score)
{
    if (m_score == score) {
        return;
    }
    m_score = std::move(score);
    onScoreChanged();
}

std::shared_ptr<MusicTrainer::music::Score> ScoreView::getScore() const
{
    return m_score;
}

QRectF ScoreView::getViewportBounds() const
{
    return m_viewportManager->getViewportBounds();
}

void ScoreView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    if (m_viewportManager) {
        m_viewportManager->updateViewSize(event->size());
        updateGridVisuals();
    }
}

void ScoreView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    if (m_viewportManager) {
        // Convert scroll position to scene coordinates
        QPointF scrollPos = mapToScene(QPoint(0, 0));
        qDebug() << "ScoreView::scrollContentsBy -"
                 << "dx:" << dx << "dy:" << dy
                 << "ScrollPos:" << scrollPos;
                 
        // ViewportManager will handle grid unit conversion
        m_viewportManager->updateScrollPosition(scrollPos);
        updateGridVisuals();
    }
}

void ScoreView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelecting = true;
        m_lastMousePos = event->pos();
    }
    QGraphicsView::mousePressEvent(event);
}

void ScoreView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isSelecting) {
        // Handle any selection or dragging in musical coordinates
        QPointF musicalPos = mapToMusicalSpace(event->pos());
        // Future implementation: handle selection rectangle or note dragging
    }
    m_lastMousePos = event->pos();
    QGraphicsView::mouseMoveEvent(event);
}

void ScoreView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isSelecting) {
        m_isSelecting = false;
        
        // Get final position in musical space
        QPointF musicalPos = mapToMusicalSpace(event->pos());
        
        // If this is a click (not a drag), potentially add a note
        if ((event->pos() - m_lastMousePos).manhattanLength() < 3) {
            // Round to nearest grid position
            int pitch = qRound(musicalPos.y());
            int position = qRound(musicalPos.x());
            // Emit signal with the note position
            emit noteAdded(pitch, 1.0, position);
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ScoreView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom
        const ViewportAnchor oldAnchor = transformationAnchor();
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        
        double factor = 1.0 + (event->angleDelta().y() / 1200.0);
        scale(factor, factor);
        
        // Update the viewport manager with new zoom level
        if (m_viewportManager) {
            float currentZoom = transform().m11(); // Get actual zoom
            m_viewportManager->updateZoomLevel(currentZoom);
            updateGridVisuals();
        }
        
        setTransformationAnchor(oldAnchor);
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void ScoreView::onScoreChanged()
{
    m_noteGrid->clear();
    if (m_score) {
        m_noteGrid->updateFromScore(m_score);
    }
    updateGridVisuals();
}

void ScoreView::onVoiceAdded(const MusicTrainer::music::Voice& voice)
{
    updateGridVisuals();
}

void ScoreView::onNoteAdded(const MusicTrainer::music::Note& note)
{
    if (m_score) {
        // Determine the position based on currently selected position or last note
        int position = 0;
        // If the score has a current position pointer, use that
        auto currentPosition = m_score->getCurrentPosition();
        if (currentPosition.has_value()) {
            position = currentPosition.value();
        }
        
        // Add the note at the determined position
        m_noteGrid->addNote(note, 0, position);  // Use voice index 0 for now
        
        // Update the current position for the next note
        // Fix: Use getDuration() instead of accessing duration directly
        m_score->setCurrentPosition(position + note.getDuration());
        
        // Check if we need to expand the viewport
        checkViewportExpansion();
        updateGridVisuals();
    } else {
        // If no score is set, just add to position 0
        m_noteGrid->addNote(note, 0, 0);
        updateGridVisuals();
    }
}

void ScoreView::checkViewportExpansion()
{
    QRectF currentBounds = m_viewportManager->getViewportBounds();
    QRectF visibleRect = mapToScene(viewport()->rect()).boundingRect();
    
    // Convert scene coordinates to musical space
    QRectF visibleMusicalRect(
        visibleRect.left() / GRID_UNIT,
        visibleRect.top() / NOTE_HEIGHT,
        visibleRect.width() / GRID_UNIT,
        visibleRect.height() / NOTE_HEIGHT
    );
    
    // Calculate potential expansion
    bool needsExpansion = false;
    QRectF newBounds = currentBounds;
    
    // Keep track of pitch range
    int currentRange = qRound(currentBounds.height());
    
    // Check vertical expansion only for octave shifts, not actual expansion
    if (visibleMusicalRect.top() <= currentBounds.top() + currentBounds.height() * 0.15) {
        // Shift the octave range up
        newBounds.translate(0, -12);
        needsExpansion = true;
    }
    else if (visibleMusicalRect.bottom() >= currentBounds.bottom() - currentBounds.height() * 0.15) {
        // Shift the octave range down
        newBounds.translate(0, 12);
        needsExpansion = true;
    }
    
    // Maintain octave height
    newBounds.setHeight(12);
    
    // Check horizontal expansion but limit to 12 measures (48 beats)
    if (visibleMusicalRect.right() >= currentBounds.right() - currentBounds.width() * 0.2) {
        float currentWidth = currentBounds.width();
        if (currentWidth < 48) { // 12 measures in 4/4 time
            float newWidth = qMin(48.0f, currentWidth + 16.0f); // Add up to 4 measures
            newBounds.setWidth(newWidth);
            needsExpansion = true;
        }
    }
    
    // Apply expansion if needed
    if (needsExpansion) {
        m_viewportManager->setViewportBounds(newBounds);
        emit viewportExpanded(newBounds);
        updateGridVisuals();
    }
}

void ScoreView::initializeViewport()
{
    // Set fixed dimensions for one octave starting at middle C
    int minPitch = 60;    // C4 (middle C)
    int maxPitch = 72;    // C5 (one octave up)
    int pitchRange = maxPitch - minPitch;
    
    int startPosition = 0;
    int endPosition = 48; // 12 measures in 4/4 time
    int beatRange = endPosition - startPosition;
    
    // Add small margin for octave labels
    float labelMargin = 25.0f;
    
    // Create grid scene rect that represents exactly one octave
    QRectF gridSceneRect(
        labelMargin,                        // Leave space for labels
        minPitch * NOTE_HEIGHT,             // Start at C4
        endPosition * GRID_UNIT,            // 12 measures
        pitchRange * NOTE_HEIGHT            // Exactly one octave
    );
    
    // Set grid dimensions
    NoteGrid::GridDimensions dimensions{
        minPitch,
        maxPitch,
        startPosition,
        endPosition
    };
    m_noteGrid->setDimensions(dimensions);
    
    // Set scene rect with minimal padding
    scene()->setSceneRect(
        0,                                              // Start at 0
        minPitch * NOTE_HEIGHT - NOTE_HEIGHT,          // Small space for labels above
        gridSceneRect.right() + labelMargin,           // Include label margin
        pitchRange * NOTE_HEIGHT + NOTE_HEIGHT * 2     // One octave plus minimal padding
    );
    
    // Set initial viewport bounds in musical space
    QRectF initialBounds(
        startPosition,
        minPitch,
        beatRange,
        pitchRange
    );
    
    m_viewportManager->setViewportBounds(initialBounds);
    m_viewportManager->updateViewSize(size());

    // Reset transform and ensure viewport is clean
    resetTransform();
    setViewportMargins(0, 0, 0, 0);
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    
    // Calculate appropriate scale to fit the grid in the viewport
    QRectF viewportRect = viewport()->rect();
    float horizontalScale = viewportRect.width() / (gridSceneRect.width() * 1.05);  // 5% margin
    float verticalScale = viewportRect.height() / (gridSceneRect.height() * 1.05);  // 5% margin
    
    // Use the smaller scale to ensure everything fits
    float scaleFactor = qMin(horizontalScale, verticalScale);
    scaleFactor = qMax(0.8f, qMin(scaleFactor, 1.5f));
    
    // Apply scale
    scale(scaleFactor, scaleFactor);
    
    // Center the view on the grid
    QPointF gridCenter(
        gridSceneRect.left() + gridSceneRect.width() * 0.5,
        gridSceneRect.center().y()
    );
    centerOn(gridCenter);
    
    // Update viewport manager with the new zoom level
    m_viewportManager->updateZoomLevel(scaleFactor);
    
    // Ensure the grid is updated
    updateGridVisuals();
}

void ScoreView::updateGridVisuals()
{
    if (!m_viewportManager || !m_noteGrid) {
        return;
    }
    
    // Get current viewport bounds in scene coordinates
    QRectF visibleSceneRect = mapToScene(viewport()->rect()).boundingRect();
    
    // Get current zoom level
    float zoom = transform().m11();
    
    qDebug() << "ScoreView::updateGridVisuals -" 
             << "VisibleScene:" << visibleSceneRect
             << "Zoom:" << zoom;
    
    // Create a musical-space representation of what's visible
    QPointF topLeft(
        visibleSceneRect.left() / GRID_UNIT,
        visibleSceneRect.top() / NOTE_HEIGHT
    );
    
    QPointF bottomRight(
        visibleSceneRect.right() / GRID_UNIT,
        visibleSceneRect.bottom() / NOTE_HEIGHT
    );
    
    QRectF musicalBounds(topLeft, bottomRight);
    
    // Clean up any previous debug visualization
    static QGraphicsRectItem* lastDebugRect = nullptr;
    if (lastDebugRect) {
        scene()->removeItem(lastDebugRect);
        delete lastDebugRect;
        lastDebugRect = nullptr;
    }
    
    // For debug visualization only - show current visible area
    if (zoom > 0.3) { // Only when zoomed in enough to see
        lastDebugRect = new QGraphicsRectItem(visibleSceneRect);
        lastDebugRect->setPen(QPen(QColor(0, 255, 0, 100), 2));  // Semi-transparent green
        lastDebugRect->setZValue(50);
        scene()->addItem(lastDebugRect);
    }
    
    // Update viewport manager with visible bounds in musical space
    m_viewportManager->setViewportBounds(musicalBounds);
    
    // Update grid with the visible scene coordinates for rendering
    auto dimensions = m_noteGrid->getDimensions();
    QRectF fullGridRect(
        dimensions.startPosition * GRID_UNIT,
        dimensions.minPitch * NOTE_HEIGHT,
        (dimensions.endPosition - dimensions.startPosition) * GRID_UNIT,
        (dimensions.maxPitch - dimensions.minPitch) * NOTE_HEIGHT
    );
    
    // Ensure we're rendering at least the full grid area
    QRectF renderArea = visibleSceneRect.united(fullGridRect);
    
    // Update the grid and grid lines with the determined area
    m_noteGrid->updateGrid(renderArea);
    m_noteGrid->updateGridLines(renderArea, zoom);
}

QPointF ScoreView::mapToMusicalSpace(const QPointF& screenPoint) const
{
    if (!m_viewportManager) {
        return QPointF();
    }
    QPointF scenePoint = mapToScene(screenPoint.toPoint());
    
    // Convert from scene coordinates to musical coordinates
    scenePoint.rx() /= GRID_UNIT;
    scenePoint.ry() /= NOTE_HEIGHT;
    
    return scenePoint;
}

QPointF ScoreView::mapFromMusicalSpace(const QPointF& musicalPoint) const
{
    if (!m_viewportManager) {
        return QPointF();
    }
    
    // Convert from musical coordinates to scene coordinates
    QPointF scenePoint(musicalPoint.x() * GRID_UNIT, musicalPoint.y() * NOTE_HEIGHT);
    return mapFromScene(scenePoint);
}

MusicTrainer::music::Duration ScoreView::convertToMusicalDuration(double numericDuration) {
    // Find the closest base duration type
    MusicTrainer::music::Duration::Type baseType = MusicTrainer::music::Duration::Type::QUARTER;
    uint8_t dots = 0;
    
    // Map of duration values to types (in beats)
    static const std::vector<std::pair<MusicTrainer::music::Duration::Type, double>> durationMap = {
        {MusicTrainer::music::Duration::Type::WHOLE, 4.0},
        {MusicTrainer::music::Duration::Type::HALF, 2.0},
        {MusicTrainer::music::Duration::Type::QUARTER, 1.0},
        {MusicTrainer::music::Duration::Type::EIGHTH, 0.5},
        {MusicTrainer::music::Duration::Type::SIXTEENTH, 0.25},
        {MusicTrainer::music::Duration::Type::THIRTY_SECOND, 0.125}
    };
    
    // Find the closest base duration
    for (const auto& [type, beats] : durationMap) {
        if (beats <= numericDuration) {
            baseType = type;
            break;
        }
    }
    
    // Add dots if needed (up to double-dotted)
    double baseValue = 4.0 / static_cast<double>(baseType);
    while (numericDuration > baseValue * 1.5 && dots < 2) {
        baseValue *= 1.5;
        dots++;
    }
    
    return MusicTrainer::music::Duration::create(baseType, dots);
}

// Fix the handleNoteAdded method to use the correct Note constructor
void ScoreView::handleNoteAdded(int pitch, double duration, int position)
{
    qDebug() << "ScoreView::handleNoteAdded -"
             << "Pitch:" << pitch
             << "Duration:" << duration
             << "Position:" << position;
             
    // Create note components using factory methods
    MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(static_cast<uint8_t>(pitch));
    MusicTrainer::music::Duration noteDuration = convertToMusicalDuration(duration);
    
    // Create a Note with the correct constructor parameters (pitch, duration)
    MusicTrainer::music::Note note(notePitch, duration, position);
             
    if (m_score) {
        // If we have a score, add it through the score's interface
        auto voice = m_score->getVoice(0); // Use first voice for now
        if (voice) {
            // Use the duration value directly instead of the Duration object
            voice->addNote(notePitch, duration, position);
            m_score->setCurrentPosition(position + duration);
        }
    }
    
    // Always add to the grid for visual representation
    // Make sure to pass the actual MIDI note number, not the position as the pitch
    m_noteGrid->addNote(note, 0, position);
    updateGridVisuals();
    checkViewportExpansion();
}

} // namespace MusicTrainer::presentation