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
    if (dx == 0 && dy == 0) return;  // Skip no-op scrolls
    
    // Call base implementation first
    QGraphicsView::scrollContentsBy(dx, dy);
    
    if (!m_viewportManager) return;

    // Convert scroll position to scene coordinates
    QPointF scrollPos = mapToScene(QPoint(0, 0));
    
    // Disable updates during scroll
    setUpdatesEnabled(false);
    
    // Update viewport manager with new scroll position
    m_viewportManager->updateScrollPosition(scrollPos);
    
    // Re-enable updates
    setUpdatesEnabled(true);
    
    // Request a single update
    viewport()->update();
}

void ScoreView::mousePressEvent(QMouseEvent *event)
{
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
        (dimensions.minPitch - 1) * NOTE_HEIGHT,
        GRID_UNIT,
        (dimensions.maxPitch - dimensions.minPitch + 2) * NOTE_HEIGHT
    );

    // Calculate the actual note placement area - exclude expansion buttons
    QRectF noteArea(
        0,  // Start from left edge
        dimensions.minPitch * NOTE_HEIGHT,  // Start at actual note area
        dimensions.endPosition * GRID_UNIT, // Full width
        (dimensions.maxPitch - dimensions.minPitch) * NOTE_HEIGHT // Only actual note area height
    );

    qDebug() << "ScoreView::mousePressEvent - Click info:"
             << "Position:" << scenePos
             << "Button:" << (event->button() == Qt::LeftButton ? "Left" : "Right")
             << "\nButtons:" 
             << "\nTop:" << topButton
             << "\nBottom:" << bottomButton
             << "\nRight:" << rightButton
             << "\nNote area:" << noteArea;

    // Handle expansion/collapse buttons
    if (topButton.contains(scenePos) || bottomButton.contains(scenePos) || rightButton.contains(scenePos)) {
        qDebug() << "ScoreView::mousePressEvent - Hit expansion button";
        if (event->button() == Qt::LeftButton) {
            // Handle expansion
            if (topButton.contains(scenePos)) {
                qDebug() << "ScoreView::mousePressEvent - Expanding up";
                m_viewportManager->expandGrid(ViewportManager::Direction::Up, 12);
                event->accept();
                return;
            } 
            if (bottomButton.contains(scenePos)) {
                qDebug() << "ScoreView::mousePressEvent - Expanding down";
                m_viewportManager->expandGrid(ViewportManager::Direction::Down, 12);
                event->accept();
                return;
            } 
            if (rightButton.contains(scenePos)) {
                qDebug() << "ScoreView::mousePressEvent - Expanding right";
                m_viewportManager->expandGrid(ViewportManager::Direction::Right, 4);
                event->accept();
                return;
            }
        }
        else if (event->button() == Qt::RightButton) {
            // Handle collapse
            if (topButton.contains(scenePos) && m_viewportManager->canCollapse(ViewportManager::Direction::Up)) {
                qDebug() << "ScoreView::mousePressEvent - Collapsing up";
                m_viewportManager->collapseGrid(ViewportManager::Direction::Up);
                event->accept();
                return;
            }
            if (bottomButton.contains(scenePos) && m_viewportManager->canCollapse(ViewportManager::Direction::Down)) {
                qDebug() << "ScoreView::mousePressEvent - Collapsing down";
                m_viewportManager->collapseGrid(ViewportManager::Direction::Down);
                event->accept();
                return;
            }
            if (rightButton.contains(scenePos) && m_viewportManager->canCollapse(ViewportManager::Direction::Right)) {
                qDebug() << "ScoreView::mousePressEvent - Collapsing right";
                m_viewportManager->collapseGrid(ViewportManager::Direction::Right);
                event->accept();
                return;
            }
        }
        qDebug() << "ScoreView::mousePressEvent - Ignoring expansion button click (no action available)";
        event->accept();
        return; // Return early if we clicked any expansion button area, even if not acted upon
    }
    
    // Handle note placement - only if we clicked in the actual note area
    if (event->button() == Qt::LeftButton && noteArea.contains(scenePos)) {
        qDebug() << "ScoreView::mousePressEvent - Starting note placement";
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
        event->accept();
        return;
    }
    
    qDebug() << "ScoreView::mousePressEvent - Passing to QGraphicsView";
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
    auto viewState = m_viewportManager->getViewportState();
    
    // Convert scene coordinates to musical space
    QRectF visibleMusicalRect(
        visibleRect.left() / GRID_UNIT,
        visibleRect.top() / NOTE_HEIGHT,
        visibleRect.width() / GRID_UNIT,
        viewState.preserveOctaveExpansion ? (visibleRect.height() / NOTE_HEIGHT) : 12  // Preserve height in multi-octave mode
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
    
    // Preserve the expanded height in multi-octave mode
    if (viewState.preserveOctaveExpansion) {
        newBounds.setHeight(currentBounds.height());  // Keep current height
    } else {
        newBounds.setHeight(12);  // Single octave height
    }
    
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
    
    // Set initial grid dimensions
    NoteGrid::GridDimensions dimensions{
        minPitch,              // C4
        minPitch + pitchRange, // C5
        startPosition,         // Start at beginning
        endPosition           // 12 measures
    };
    m_noteGrid->setDimensions(dimensions);
    
    // Set scene rect to exactly match the current octave plus margins
    scene()->setSceneRect(
        -labelMargin,  // Space for labels
        dimensions.minPitch * NOTE_HEIGHT - NOTE_HEIGHT * 0.5,  // Small padding above
        endPosition * GRID_UNIT + 2 * labelMargin,  // Grid width plus margins
        (pitchRange + 1) * NOTE_HEIGHT  // One octave plus padding
    );
    
    // Set initial viewport bounds in musical space
    QRectF initialBounds(
        startPosition,
        dimensions.minPitch,
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
    float horizontalScale = viewportRect.width() / (endPosition * GRID_UNIT * 1.05);
    float verticalScale = viewportRect.height() / (pitchRange * NOTE_HEIGHT * 1.05);
    float scaleFactor = qMin(horizontalScale, verticalScale);
    scaleFactor = qMax(0.8f, qMin(scaleFactor, 1.5f));
    
    // Apply scale
    scale(scaleFactor, scaleFactor);
    
    // Center on the grid
    centerOn(QPointF(endPosition * GRID_UNIT / 2.0,
                    (dimensions.minPitch + pitchRange / 2.0) * NOTE_HEIGHT));
    
    // Update viewport manager
    m_viewportManager->updateZoomLevel(scaleFactor);
    updateGridVisuals();
}

void ScoreView::updateGridVisuals()
{
    if (!m_viewportManager || !m_noteGrid) return;
    
    // Get current grid dimensions
    auto dimensions = m_noteGrid->getDimensions();
    
    // Get current viewport state to check if we're in multi-octave mode
    auto viewState = m_viewportManager->getViewportState();
    
    // Get visible viewport area in scene coordinates
    QRectF viewportSceneRect = mapToScene(viewport()->rect()).boundingRect();
    
    // Convert viewport rect to musical coordinates
    QRectF musicalBounds;
    
    if (viewState.preserveOctaveExpansion) {
        // In multi-octave mode, preserve the full expanded range
        musicalBounds = QRectF(
            qMax(0.0, viewportSceneRect.x() / GRID_UNIT),
            dimensions.minPitch,  // Use the full expanded range
            viewportSceneRect.width() / GRID_UNIT,
            dimensions.maxPitch - dimensions.minPitch  // Use actual pitch range
        );
    } else {
        // In single-octave mode, force exactly one octave
        musicalBounds = QRectF(
            qMax(0.0, viewportSceneRect.x() / GRID_UNIT),
            dimensions.minPitch,  // Lock to current octave
            viewportSceneRect.width() / GRID_UNIT,
            12  // Force octave height
        );
    }
    
    // Update viewport manager with visible area but don't reset dimensions
    // Pass false for resetDimensions to maintain the expanded state
    m_viewportManager->setViewportBounds(musicalBounds);
    
    // Only update the grid for the visible area plus a small buffer
    QRectF updateRect = viewportSceneRect.adjusted(-GRID_UNIT, -NOTE_HEIGHT, GRID_UNIT, NOTE_HEIGHT);
    m_noteGrid->updateGrid(updateRect);
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
    if (!m_noteGrid || !m_viewportManager) return;

    auto dimensions = m_noteGrid->getDimensions();
    auto viewState = m_viewportManager->getViewportState();
    
    // Clamp the pitch to the current viewport bounds
    pitch = qBound(dimensions.minPitch, pitch, dimensions.maxPitch - 1);
    position = qBound(dimensions.startPosition, position, dimensions.endPosition - 1);
    
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
    
    // Only update the grid for the new note's area
    QRectF updateRect(
        position * GRID_UNIT - GRID_UNIT,  // Include one unit before
        pitch * NOTE_HEIGHT - NOTE_HEIGHT,  // Include one unit above
        GRID_UNIT * 2,  // Two units wide
        NOTE_HEIGHT * 2  // Two units high
    );
    m_noteGrid->updateGrid(updateRect);
}

void ScoreView::expandGrid(ViewportManager::Direction direction, int amount)
{
    if (!m_viewportManager || !m_noteGrid) return;

    // Store viewport state before expansion
    QPointF oldCenter = mapToScene(viewport()->rect().center());
    QPointF oldCorner = mapToScene(QPoint(0, 0));
    
    // Block viewport updates during expansion
    setUpdatesEnabled(false);
    
    // Perform grid expansion
    m_viewportManager->expandGrid(direction, amount);
    
    // Get new dimensions
    auto dimensions = m_noteGrid->getDimensions();
    
    // Calculate new scene rect with margins
    float labelMargin = 25.0f;
    QRectF newSceneRect(
        -labelMargin,  // Space for labels
        dimensions.minPitch * NOTE_HEIGHT - NOTE_HEIGHT * 0.5f,  // Small padding above
        dimensions.endPosition * GRID_UNIT + 2 * labelMargin,  // Width plus margins
        (dimensions.maxPitch - dimensions.minPitch + 2) * NOTE_HEIGHT  // Full height of expanded octaves plus buttons
    );
    
    // Update scene rect
    scene()->setSceneRect(newSceneRect);

    // Calculate the new center based on expansion direction
    QPointF newCenter;
    if (direction == ViewportManager::Direction::Up) {
        // For upward expansion, keep view centered on the original octave
        // but adjust the scene to show the additional octave above
        newCenter = QPointF(
            oldCenter.x(),
            oldCenter.y() + 6 * NOTE_HEIGHT  // Move down to keep original octave visible
        );
    } 
    else if (direction == ViewportManager::Direction::Down) {
        // For downward expansion, keep center on the original position
        // since the new octave is added below
        newCenter = oldCenter;
    } 
    else {
        // For horizontal expansion, keep the same center position
        newCenter = oldCenter;
    }

    // Re-enable updates before changing viewport
    setUpdatesEnabled(true);
    
    // Center on the new position
    centerOn(newCenter);
    
    // Update scroll position in viewport manager
    QPointF newScrollPos = mapToScene(QPoint(0, 0));
    m_viewportManager->updateScrollPosition(newScrollPos);
    
    // Force a complete update
    viewport()->update();
    scene()->update(scene()->sceneRect());
    
    // Notify any listeners of the expansion
    emit viewportExpanded(m_viewportManager->getViewportBounds());
}

} // namespace MusicTrainer::presentation