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
    
    // Set up scene coordinate system
    scene()->setSceneRect(-1000, -1000, 2000, 2000); // Large initial scene rect
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
    
    // Calculate potential expansion
    bool needsExpansion = false;
    QRectF newBounds = currentBounds;
    
    // Check vertical expansion needs
    if (visibleRect.top() <= currentBounds.top() + currentBounds.height() * 0.1) {
        // Expand upward
        newBounds.setTop(currentBounds.top() - 12); // One octave
        needsExpansion = true;
    }
    if (visibleRect.bottom() >= currentBounds.bottom() - currentBounds.height() * 0.1) {
        // Expand downward
        newBounds.setBottom(currentBounds.bottom() + 12); // One octave
        needsExpansion = true;
    }
    
    // Check horizontal expansion needs
    if (visibleRect.right() >= currentBounds.right() - currentBounds.width() * 0.2) {
        // Expand rightward
        newBounds.setRight(currentBounds.right() + 16); // Additional measures
        needsExpansion = true;
    }
    
    if (needsExpansion) {
        m_viewportManager->setViewportBounds(newBounds);
        emit viewportExpanded(newBounds);
    }
}

void ScoreView::initializeViewport()
{
    // Center around middle C (MIDI note 60) with a range of 4 octaves (48 semitones)
    // This sets the bounds from MIDI note 36 (C2) to MIDI note 84 (C6)
    QRectF initialBounds(0, 36, 32, 48);
    
    qDebug() << "ScoreView::initializeViewport -"
             << "InitialBounds:" << initialBounds
             << "Size:" << size()
             << "GRID_UNIT:" << GRID_UNIT
             << "NOTE_HEIGHT:" << NOTE_HEIGHT;
             
    m_viewportManager->setViewportBounds(initialBounds);
    
    // Initialize viewport state
    m_viewportManager->updateViewSize(size());
    m_viewportManager->updateZoomLevel(GRID_ZOOM_BASE);
    
    // Reset the scroll position to ensure we're at the origin
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    
    // Calculate the center position in musical space:
    // Center on C4 (MIDI note 60) and measure 1 (position 0)
    float centerX = 0;  // Start of the grid (measure 1)
    float centerY = 60; // Middle C (MIDI note 60)
    
    // Convert to scene coordinates
    float sceneCenterX = centerX * GRID_UNIT;
    float sceneCenterY = centerY * NOTE_HEIGHT;
    
    // Center the view on this position
    centerOn(sceneCenterX, sceneCenterY);
    
    // Force an immediate update of grid visuals
    updateGridVisuals();
}

void ScoreView::updateGridVisuals()
{
    if (!m_viewportManager || !m_noteGrid) {
        return;
    }
    
    QRectF bounds = m_viewportManager->getViewportBounds();
    float zoom = transform().m11(); // Get actual zoom without grid unit scaling
    
    qDebug() << "ScoreView::updateGridVisuals -" 
             << "Bounds:" << bounds
             << "Zoom:" << zoom
             << "Transform:" << transform()
             << "ViewportRect:" << viewport()->rect()
             << "SceneRect:" << sceneRect();
    
    // Scale bounds to scene coordinates for the grid
    QRectF scaledBounds = bounds;
    scaledBounds.setLeft(bounds.left() * GRID_UNIT);
    scaledBounds.setRight(bounds.right() * GRID_UNIT);
    scaledBounds.setTop(bounds.top() * NOTE_HEIGHT);
    scaledBounds.setBottom(bounds.bottom() * NOTE_HEIGHT);
    
    // Update grid with properly scaled bounds
    m_noteGrid->updateGrid(scaledBounds);
    m_noteGrid->updateGridLines(scaledBounds, zoom);
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
    // Create note components using factory methods
    MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(static_cast<uint8_t>(pitch));
    MusicTrainer::music::Duration noteDuration = convertToMusicalDuration(duration);
    
    // Create a Note with the correct constructor parameters (pitch, duration, position)
    MusicTrainer::music::Note note(notePitch, duration, position);

    qDebug() << "ScoreView::handleNoteAdded -"
             << "Pitch:" << pitch
             << "Duration:" << duration
             << "Position:" << position;
             
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
    m_noteGrid->addNote(note, 0, position);
    updateGridVisuals();
    checkViewportExpansion();
}

} // namespace MusicTrainer::presentation