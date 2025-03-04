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
    ::registerQtTypes();

    setScene(new QGraphicsScene(this));
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    
    // Create note grid first with initial octave (C4-C5)
    m_noteGrid = std::make_unique<NoteGrid>(scene());
    
    // Create viewport manager with initial one-octave constraint
    m_viewportManager = std::make_unique<ViewportManager>(m_noteGrid.get());
    
    // Set up minimal scene rect until proper initialization
    scene()->setSceneRect(0, 0, 100, 100);
    
    // Connect signals after grid and viewport manager are created
    connect(this, &ScoreView::noteAdded, this, &ScoreView::handleNoteAdded);
    
    // Initialize viewport last to ensure all components are ready
    QTimer::singleShot(0, this, [this]() {
        initializeViewport();
    });
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
    // Call base implementation first
    QGraphicsView::scrollContentsBy(dx, dy);
    
    if (m_viewportManager) {
        // Convert scroll position to scene coordinates
        QPointF oldCenter = mapToScene(viewport()->rect().center());
        QPointF scrollPos = mapToScene(QPoint(0, 0));
        
        qDebug() << "ScoreView::scrollContentsBy -"
                 << "dx:" << dx << "dy:" << dy
                 << "ScrollPos:" << scrollPos
                 << "Center:" << oldCenter;
                 
        // Update viewport manager with new scroll position
        m_viewportManager->updateScrollPosition(scrollPos);
        
        // Update grid visuals with current view center
        updateGridVisuals();
        
        // Maintain view center after grid update
        centerOn(oldCenter);
    }
}

void ScoreView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        auto dimensions = m_noteGrid->getDimensions();
        
        // Calculate expansion button regions in scene coordinates
        QRectF topButton(
            0,  // Start from left edge
            (dimensions.minPitch - 1) * NOTE_HEIGHT,
            dimensions.endPosition * GRID_UNIT,
            NOTE_HEIGHT
        );
        
        QRectF bottomButton(
            0,  // Start from left edge
            dimensions.maxPitch * NOTE_HEIGHT,
            dimensions.endPosition * GRID_UNIT,
            NOTE_HEIGHT
        );
        
        QRectF rightButton(
            dimensions.endPosition * GRID_UNIT,
            (dimensions.minPitch - 1) * NOTE_HEIGHT,  // Include expansion areas
            GRID_UNIT,
            (dimensions.maxPitch - dimensions.minPitch + 2) * NOTE_HEIGHT
        );

        // Check if click was in expansion buttons and handle them exclusively
        if (topButton.contains(scenePos)) {
            m_viewportManager->expandGrid(ViewportManager::Direction::Up, 12);
            event->accept();
            return;
        } 
        if (bottomButton.contains(scenePos)) {
            m_viewportManager->expandGrid(ViewportManager::Direction::Down, 12);
            event->accept();
            return;
        } 
        if (rightButton.contains(scenePos)) {
            m_viewportManager->expandGrid(ViewportManager::Direction::Right, 4);
            event->accept();
            return;
        }
        
        // Convert scene position to musical coordinates for preview
        QPointF musicalPos = mapToMusicalSpace(event->pos());
        
        // Quantize position and show preview
        double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
        int position = qBound(dimensions.startPosition,
                            static_cast<int>(quantizedPosition),
                            dimensions.endPosition - 1);
        int pitch = qBound(dimensions.minPitch,
                          qRound(musicalPos.y()),
                          dimensions.maxPitch - 1);
                          
        m_noteGrid->showNotePreview(position, pitch);
        
        // Start note placement
        m_isSelecting = true;
        m_lastMousePos = event->pos();
    }
    QGraphicsView::mousePressEvent(event);
}

void ScoreView::mouseMoveEvent(QMouseEvent *event)
{
    // Get position in musical space
    QPointF musicalPos = mapToMusicalSpace(event->pos());
    
    if (m_isSelecting) {
        // Get current grid dimensions
        auto dimensions = m_noteGrid->getDimensions();
        
        // Quantize the position to nearest grid line
        double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
        int position = qBound(dimensions.startPosition,
                            static_cast<int>(quantizedPosition),
                            dimensions.endPosition - 1);
                            
        // Round pitch to nearest semitone and clamp to valid range
        int pitch = qBound(dimensions.minPitch,
                          qRound(musicalPos.y()),
                          dimensions.maxPitch - 1);
        
        // Show preview at quantized position
        m_noteGrid->showNotePreview(position, pitch);
    }
    
    m_lastMousePos = event->pos();
    QGraphicsView::mouseMoveEvent(event);
}

void ScoreView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isSelecting) {
        m_isSelecting = false;
        
        // Hide the preview
        m_noteGrid->hideNotePreview();
        
        // Get final position in musical space
        QPointF musicalPos = mapToMusicalSpace(event->pos());
        
        // If this is a click (not a drag), potentially add a note
        if ((event->pos() - m_lastMousePos).manhattanLength() < 3) {
            // Get current grid dimensions
            auto dimensions = m_noteGrid->getDimensions();
            
            // Enhanced precision: Calculate exact grid position with proper rounding
            // For pitch, round to the nearest semitone (integer)
            int pitch = qBound(dimensions.minPitch, 
                             qRound(musicalPos.y()), 
                             dimensions.maxPitch - 1);
                             
            // For horizontal position, use more precise quantization based on grid units
            // Quantize to the nearest quarter note by default
            double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
            int position = qBound(dimensions.startPosition,
                                static_cast<int>(quantizedPosition),
                                dimensions.endPosition - 1);
                                
            qDebug() << "ScoreView::mouseReleaseEvent - Adding note at"
                     << "Position:" << position
                     << "Pitch:" << pitch
                     << "Raw musical position:" << musicalPos
                     << "Quantized position:" << quantizedPosition;
                                
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
        12  // Force one octave height
    );
    
    // Calculate potential expansion
    bool needsExpansion = false;
    QRectF newBounds = currentBounds;
    
    // Only allow octave shifts when near edges
    float edgeThreshold = 0.5; // Half semitone from edge
    if (visibleMusicalRect.top() <= currentBounds.top() + edgeThreshold) {
        // Shift up one octave
        newBounds.translate(0, -12);
        needsExpansion = true;
    }
    else if (visibleMusicalRect.bottom() >= currentBounds.bottom() - edgeThreshold) {
        // Shift down one octave
        newBounds.translate(0, 12);
        needsExpansion = true;
    }
    
    // Always maintain one octave height
    newBounds.setHeight(12);
    
    // Check horizontal expansion but limit to 12 measures
    if (visibleMusicalRect.right() >= currentBounds.right() - 2) { // Within 2 beats of right edge
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
    int pitchRange = 12;  // Force exactly one octave
    
    int startPosition = 0;
    int endPosition = 48; // 12 measures in 4/4 time
    
    // Add small margin for octave labels (C4, etc.)
    float labelMargin = 25.0f;
    
    // Create initial grid scene rect
    QRectF gridSceneRect(
        labelMargin,                    // Space for labels
        minPitch * NOTE_HEIGHT,         // Start at C4
        endPosition * GRID_UNIT,        // 12 measures
        pitchRange * NOTE_HEIGHT        // Exactly one octave
    );
    
    // Set initial grid dimensions
    NoteGrid::GridDimensions dimensions{
        minPitch,              // C4
        minPitch + pitchRange, // C5
        startPosition,         // Start at beginning
        endPosition           // 12 measures
    };
    m_noteGrid->setDimensions(dimensions);
    
    // Set scene rect with minimal padding
    scene()->setSceneRect(
        0,                                  // Start at 0
        minPitch * NOTE_HEIGHT - NOTE_HEIGHT * 0.5,  // Small space above
        gridSceneRect.right() + labelMargin,         // Include label margin
        pitchRange * NOTE_HEIGHT + NOTE_HEIGHT       // One octave plus minimal padding
    );
    
    // Set initial viewport bounds in musical space
    QRectF initialBounds(
        startPosition,
        minPitch,
        endPosition - startPosition,
        pitchRange  // Force one octave height
    );
    
    m_viewportManager->setViewportBounds(initialBounds);
    m_viewportManager->updateViewSize(size());

    // Reset transform and scrollbars
    resetTransform();
    setViewportMargins(0, 0, 0, 0);
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    
    // Calculate scale to fit the viewport
    QRectF viewportRect = viewport()->rect();
    float horizontalScale = viewportRect.width() / (gridSceneRect.width() * 1.05);
    float verticalScale = viewportRect.height() / (gridSceneRect.height() * 1.05);
    float scaleFactor = qMin(horizontalScale, verticalScale);
    scaleFactor = qMax(0.8f, qMin(scaleFactor, 1.5f));
    
    // Apply scale
    scale(scaleFactor, scaleFactor);
    
    // Center on the grid
    centerOn(gridSceneRect.center());
    
    // Update viewport manager
    m_viewportManager->updateZoomLevel(scaleFactor);
    updateGridVisuals();
}

void ScoreView::updateGridVisuals()
{
    if (!m_viewportManager || !m_noteGrid) {
        return;
    }
    
    // Get current grid dimensions
    auto dimensions = m_noteGrid->getDimensions();
    
    // Calculate scene rect size with padding for labels and expansion buttons
    float labelMargin = 25.0f;  // Space for labels on left
    float buttonPadding = NOTE_HEIGHT;  // Space for expansion buttons
    
    // Create scene rect that includes the entire grid plus margins
    QRectF sceneRect(
        -labelMargin,  // Add space for labels on the left
        (dimensions.minPitch - 1) * NOTE_HEIGHT,  // One note height above for top expansion
        (dimensions.endPosition + 1) * GRID_UNIT + 2 * labelMargin,  // Grid width plus margins
        (dimensions.maxPitch - dimensions.minPitch + 2) * NOTE_HEIGHT  // Grid height plus expansion buttons
    );
    
    // Update scene rect
    scene()->setSceneRect(sceneRect);
    
    // Get current viewport rect in scene coordinates
    QRectF viewportSceneRect = mapToScene(viewport()->rect()).boundingRect();
    
    // Create grid rect based on dimensions
    QRectF gridRect(
        0,  // Grid starts at 0
        dimensions.minPitch * NOTE_HEIGHT,
        dimensions.endPosition * GRID_UNIT,
        (dimensions.maxPitch - dimensions.minPitch) * NOTE_HEIGHT
    );
    
    // Convert viewport rect to musical coordinates for viewport manager
    QRectF musicalBounds(
        qMax(0.0, viewportSceneRect.x() / GRID_UNIT),
        dimensions.minPitch,  // Lock to current octave
        viewportSceneRect.width() / GRID_UNIT,
        12  // Force octave height
    );
    
    // Update viewport manager
    m_viewportManager->setViewportBounds(musicalBounds);
    
    // Update grid visuals
    m_noteGrid->updateGrid(gridRect);
}

QPointF ScoreView::mapToMusicalSpace(const QPointF& screenPoint) const
{
    if (!m_viewportManager) {
        return QPointF();
    }
    
    QPointF scenePoint = mapToScene(screenPoint.toPoint());
    
    // Get current grid dimensions for proper mapping
    auto dimensions = m_noteGrid->getDimensions();
    
    // Convert from scene coordinates to musical coordinates with higher precision
    double musicalX = scenePoint.x() / GRID_UNIT;
    double musicalY = scenePoint.y() / NOTE_HEIGHT;
    
    qDebug() << "ScoreView::mapToMusicalSpace -"
             << "Screen:" << screenPoint
             << "Scene:" << scenePoint
             << "Musical:" << QPointF(musicalX, musicalY);
    
    return QPointF(musicalX, musicalY);
}

QPointF ScoreView::mapFromMusicalSpace(const QPointF& musicalPoint) const
{
    if (!m_viewportManager) {
        return QPointF();
    }
    
    // Convert from musical coordinates to scene coordinates
    double sceneX = musicalPoint.x() * GRID_UNIT;
    double sceneY = musicalPoint.y() * NOTE_HEIGHT;
    
    return mapFromScene(QPointF(sceneX, sceneY));
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
             
    // Create note components directly - pitch is already a valid MIDI note number
    MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(pitch);
    MusicTrainer::music::Duration noteDuration = convertToMusicalDuration(duration);
    
    // Create a Note object with the converted pitch
    MusicTrainer::music::Note note(notePitch, noteDuration.getTotalBeats(), position);
             
    if (m_score) {
        // If we have a score, add it through the score's interface
        auto voice = m_score->getVoice(0); // Use first voice for now
        if (voice) {
            voice->addNote(notePitch, duration, position);
            m_score->setCurrentPosition(position + duration);
        }
    }
    
    // Add to grid for visual representation
    m_noteGrid->addNote(note, 0, position);
    updateGridVisuals();
    checkViewportExpansion();
}

void ScoreView::expandGrid(ViewportManager::Direction direction, int amount)
{
    if (!m_viewportManager) return;

    // Store scroll position and center before expansion
    QPointF oldCenter = mapToScene(viewport()->rect().center());
    QPointF oldCorner = mapToScene(QPoint(0, 0));
    
    // Block viewport updates during expansion
    setUpdatesEnabled(false);
    
    // Perform grid expansion
    m_viewportManager->expandGrid(direction, amount);
    
    // Re-enable updates
    setUpdatesEnabled(true);
    
    // Adjust viewport to maintain musical position
    QPointF newCenter;
    if (direction == ViewportManager::Direction::Up || 
        direction == ViewportManager::Direction::Down) {
        // For vertical expansion, maintain the same horizontal position
        auto dims = m_noteGrid->getDimensions();
        newCenter = QPointF(
            oldCenter.x(),
            (dims.minPitch + 6) * NOTE_HEIGHT  // Center vertically in new octave
        );
        centerOn(newCenter);
    } else {
        // For horizontal expansion, maintain the same center
        centerOn(oldCenter);
    }
    
    // Force a full update
    viewport()->update();
    scene()->update();
}

} // namespace MusicTrainer::presentation