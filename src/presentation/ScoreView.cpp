#include "presentation/ScoreView.h"
#include "presentation/ViewportManager.h"
#include "presentation/NoteGrid.h"
#include "presentation/QtTypeRegistration.h"
#include "domain/music/Note.h"
#include "domain/music/Score.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/events/NoteAddedEvent.h"
#include <QScrollBar>
#include <QResizeEvent>
#include <QGraphicsRectItem>
#include <QWheelEvent>
#include <QTimer>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>
#include <QStyle>
#include <QElapsedTimer>

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
    
    // Set up duration toolbar before scene setup
    setupDurationToolbar();

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
    
    // Create expansion button rectangles and visual items if they don't exist
    if (!m_topExpandButton) {
        m_topExpandButton = new QGraphicsRectItem(0, 0, 0, NOTE_HEIGHT);
        m_topExpandButton->setPen(QPen(Qt::black));
        m_topExpandButton->setBrush(QBrush(QColor(200, 200, 255, 100)));
        m_topExpandButton->setZValue(1000); // Ensure it's above the grid
        scene()->addItem(m_topExpandButton);
    }
    if (!m_bottomExpandButton) {
        m_bottomExpandButton = new QGraphicsRectItem(0, 0, 0, NOTE_HEIGHT);
        m_bottomExpandButton->setPen(QPen(Qt::black));
        m_bottomExpandButton->setBrush(QBrush(QColor(200, 200, 255, 100)));
        m_bottomExpandButton->setZValue(1000);
        scene()->addItem(m_bottomExpandButton);
    }
    if (!m_rightExpandButton) {
        m_rightExpandButton = new QGraphicsRectItem(0, 0, GRID_UNIT, 0);
        m_rightExpandButton->setPen(QPen(Qt::black));
        m_rightExpandButton->setBrush(QBrush(QColor(200, 200, 255, 100)));
        m_rightExpandButton->setZValue(1000);
        scene()->addItem(m_rightExpandButton);
    }
    
    // Update button positions and sizes - Fix the top button position to have more separation
    QRectF topButton(
        0,
        (dimensions.minPitch - 1.5) * NOTE_HEIGHT, // Increased separation by moving it up 0.5 NOTE_HEIGHT
        dimensions.endPosition * GRID_UNIT,
        NOTE_HEIGHT
    );
    m_topExpandButton->setRect(topButton);
    
    QRectF bottomButton(
        0,
        dimensions.maxPitch * NOTE_HEIGHT,
        dimensions.endPosition * GRID_UNIT,
        NOTE_HEIGHT
    );
    m_bottomExpandButton->setRect(bottomButton);
    
    QRectF rightButton(
        dimensions.endPosition * GRID_UNIT,
        (dimensions.minPitch - 1.5) * NOTE_HEIGHT, // Also adjust the top of the right button to align with the top button
        GRID_UNIT,
        (dimensions.maxPitch - dimensions.minPitch + 2.5) * NOTE_HEIGHT // Adjust height to accommodate the new top position
    );
    m_rightExpandButton->setRect(rightButton);

    QRectF noteArea(
        0,
        dimensions.minPitch * NOTE_HEIGHT,
        dimensions.endPosition * GRID_UNIT,
        (dimensions.maxPitch - dimensions.minPitch) * NOTE_HEIGHT
    );

    // Handle expansion/collapse buttons first
    if (topButton.contains(scenePos) || bottomButton.contains(scenePos) || rightButton.contains(scenePos)) {
        auto viewState = m_viewportManager->getViewportState();
        
        // Check if we're in multi-octave mode directly by examining the pitch range
        bool isMultiOctave = (dimensions.maxPitch - dimensions.minPitch > 12);

        if (event->button() == Qt::LeftButton) {
            // Handle expansion
            if (topButton.contains(scenePos)) {
                expandGrid(ViewportManager::Direction::Up, 12);
            } else if (bottomButton.contains(scenePos)) {
                expandGrid(ViewportManager::Direction::Down, 12);
            } else if (rightButton.contains(scenePos)) {
                expandGrid(ViewportManager::Direction::Right, 4);
            }
        } else if (event->button() == Qt::RightButton && isMultiOctave) {
            // Handle collapse only if in multi-octave mode
            if (topButton.contains(scenePos)) {
                m_viewportManager->collapseGrid(ViewportManager::Direction::Up);
                updateGridVisuals(); // Ensure visuals are updated after collapse
            } else if (bottomButton.contains(scenePos)) {
                m_viewportManager->collapseGrid(ViewportManager::Direction::Down);
                updateGridVisuals();
            } else if (rightButton.contains(scenePos)) {
                m_viewportManager->collapseGrid(ViewportManager::Direction::Right);
                updateGridVisuals();
            }
        }
        return;
    }

    // Handle note area interactions
    if (noteArea.contains(scenePos)) {
        QPointF musicalPos = mapToMusicalSpace(event->pos());
        
        // Quantize position and pitch
        double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
        int position = qBound(dimensions.startPosition,
                            static_cast<int>(quantizedPosition),
                            dimensions.endPosition - 1);
        int pitch = qBound(dimensions.minPitch,
                          qRound(musicalPos.y()),
                          dimensions.maxPitch - 1);
        
        if (event->button() == Qt::LeftButton) {
            // Check if clicking on an existing note
            if (m_noteGrid->hasNoteAt(position, pitch)) {
                qDebug() << "ScoreView::mousePressEvent - Starting note drag";
                startNoteDrag(scenePos);
            } else {
                qDebug() << "ScoreView::mousePressEvent - Starting note placement";
                m_isSelecting = true;
                m_noteGrid->showNotePreview(position, pitch, m_currentNoteDuration);
            }
        } 
        else if (event->button() == Qt::RightButton) {
            if (m_noteGrid->hasNoteAt(position, pitch)) {
                qDebug() << "ScoreView::mousePressEvent - Deleting note";
                deleteNoteAt(scenePos);
            }
            // Removed cycling duration functionality on right-click
        }
        
        m_lastMousePos = event->pos();
        event->accept();
        return;
    }
    
    QGraphicsView::mousePressEvent(event);
}

void ScoreView::mouseMoveEvent(QMouseEvent *event)
{
    // Get position in musical space
    QPointF musicalPos = mapToMusicalSpace(event->pos());
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
    
    if (m_isDragging) {
        updateNoteDrag(mapToScene(event->pos()));
    }
    else if (m_isSelecting) {
        // Show preview at quantized position with current duration
        m_noteGrid->showNotePreview(position, pitch, m_currentNoteDuration);
    }
    
    m_lastMousePos = event->pos();
    QGraphicsView::mouseMoveEvent(event);
}

void ScoreView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isDragging) {
            finishNoteDrag(mapToScene(event->pos()));
            m_isDragging = false;
        }
        else if (m_isSelecting) {
            m_isSelecting = false;
            
            // Hide the preview
            m_noteGrid->hideNotePreview();
            
            // Get final position in musical space
            QPointF musicalPos = mapToMusicalSpace(event->pos());
            
            // If this is a click (not a drag), potentially add a note
            if ((event->pos() - m_lastMousePos).manhattanLength() < 3) {
                // Get current grid dimensions
                auto dimensions = m_noteGrid->getDimensions();
                
                // For pitch, round to the nearest semitone (integer)
                int pitch = qBound(dimensions.minPitch, 
                                 qRound(musicalPos.y()), 
                                 dimensions.maxPitch - 1);
                
                // Quantize to the nearest quarter note by default
                double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
                int position = qBound(dimensions.startPosition,
                                    static_cast<int>(quantizedPosition),
                                    dimensions.endPosition - 1);
                
                qDebug() << "ScoreView::mouseReleaseEvent - Adding note at"
                         << "Position:" << position
                         << "Pitch:" << pitch
                         << "Duration:" << m_currentNoteDuration
                         << "Raw musical position:" << musicalPos
                         << "Quantized position:" << quantizedPosition;
                
                // Emit signal with the note position and current duration
                emit noteAdded(pitch, m_currentNoteDuration, position);
            }
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
        newCenter = oldCorner;
    } 
    else {
        // For horizontal expansion, keep the same center position
        newCenter = oldCorner;
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

void ScoreView::deleteNoteAt(const QPointF& scenePos)
{
    if (!m_noteGrid || !m_score) return;
    
    // Add rate limiting for rapid deletes
    static QElapsedTimer lastDeleteTime;
    if (lastDeleteTime.isValid() && lastDeleteTime.elapsed() < 50) {
        return;
    }
    lastDeleteTime.restart();
    
    QPointF musicalPos = mapToMusicalSpace(mapFromScene(scenePos));
    int position = static_cast<int>(qRound(musicalPos.x() * 4.0) / 4.0);
    int pitch = qRound(musicalPos.y());
    
    // Remove note from the score
    auto voice = m_score->getVoice(0); // Using first voice for now
    if (voice && m_noteGrid->hasNoteAt(position, pitch)) {
        qDebug() << "ScoreView::deleteNoteAt - Deleting note at position:" << position;
        
        // Remove from both voice and grid directly to avoid full update
        voice->removeNote(position);
        m_noteGrid->removeNotesInRange(position, position + 1);
        
        // Only update the immediate area around the deleted note
        QRectF updateRect(
            position * GRID_UNIT - GRID_UNIT,  // Include one unit before
            pitch * NOTE_HEIGHT - NOTE_HEIGHT,  // Include one unit above
            GRID_UNIT * 2,  // Two units wide
            NOTE_HEIGHT * 2  // Two units high
        );
        m_noteGrid->updateGrid(updateRect);
    }
}

void ScoreView::startNoteDrag(const QPointF& scenePos)
{
    if (!m_noteGrid) return;
    
    QPointF musicalPos = mapToMusicalSpace(mapFromScene(scenePos));
    m_draggedNotePosition = static_cast<int>(qRound(musicalPos.x() * 4.0) / 4.0);
    m_draggedNotePitch = qRound(musicalPos.y());
    
    if (m_noteGrid->hasNoteAt(m_draggedNotePosition, m_draggedNotePitch)) {
        m_isDragging = true;
        if (m_score) {
            auto voice = m_score->getVoice(0);
            if (voice) {
                if (auto note = voice->getNoteAt(m_draggedNotePosition)) {
                    // Show preview with correct duration
                    m_noteGrid->showNotePreview(m_draggedNotePosition, m_draggedNotePitch, note->getDuration());
                }
            }
        }
    }
}

void ScoreView::updateNoteDrag(const QPointF& scenePos)
{
    if (!m_isDragging || !m_noteGrid || !m_score) return;
    
    QPointF musicalPos = mapToMusicalSpace(mapFromScene(scenePos));
    auto dimensions = m_noteGrid->getDimensions();
    
    // Get current duration of the note being dragged
    double duration = m_currentNoteDuration;
    auto voice = m_score->getVoice(0);
    if (voice) {
        if (auto note = voice->getNoteAt(m_draggedNotePosition)) {
            duration = note->getDuration();
        }
    }
    
    // Quantize new position
    double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
    int newPosition = qBound(dimensions.startPosition,
                           static_cast<int>(quantizedPosition),
                           dimensions.endPosition - 1);
    
    int newPitch = qBound(dimensions.minPitch,
                         qRound(musicalPos.y()),
                         dimensions.maxPitch - 1);
    
    // Update preview with correct duration
    m_noteGrid->showNotePreview(newPosition, newPitch, duration);
}

void ScoreView::finishNoteDrag(const QPointF& scenePos)
{
    if (!m_isDragging || !m_noteGrid || !m_score) return;
    
    QPointF musicalPos = mapToMusicalSpace(mapFromScene(scenePos));
    auto dimensions = m_noteGrid->getDimensions();
    
    // Get new position
    double quantizedPosition = qRound(musicalPos.x() * 4.0) / 4.0;
    int newPosition = qBound(dimensions.startPosition,
                           static_cast<int>(quantizedPosition),
                           dimensions.endPosition - 1);
    
    int newPitch = qBound(dimensions.minPitch,
                         qRound(musicalPos.y()),
                         dimensions.maxPitch - 1);
    
    // Only move if position actually changed
    if (newPosition != m_draggedNotePosition || newPitch != m_draggedNotePitch) {
        auto voice = m_score->getVoice(0); // Using first voice for now
        if (voice) {
            // Get the note's duration before removing it
            double duration = m_currentNoteDuration;
            if (auto note = voice->getNoteAt(m_draggedNotePosition)) {
                duration = note->getDuration();
            }
            
            // Remove from old position and add at new position
            voice->removeNote(m_draggedNotePosition);
            voice->addNote(MusicTrainer::music::Pitch::fromMidiNote(newPitch),
                         duration, newPosition);
        }
    }
    
    // Clean up drag state
    m_isDragging = false;
    m_draggedNotePosition = -1;
    m_draggedNotePitch = -1;
    m_noteGrid->hideNotePreview();
    
    // Update display
    m_noteGrid->updateFromScore(m_score);
    updateGridVisuals();
}

void ScoreView::cycleDuration()
{
    // Find next duration button in sequence
    auto buttons = m_durationButtons->buttons();
    if (buttons.isEmpty()) return;
    
    // Find current button
    QAbstractButton* currentButton = nullptr;
    for (auto* button : buttons) {
        if (button->property("duration").toDouble() == m_currentNoteDuration) {
            currentButton = button;
            break;
        }
    }
    
    // Get next button, wrapping around to first
    QAbstractButton* nextButton = currentButton ? buttons.at((buttons.indexOf(currentButton) + 1) % buttons.size())
                                              : buttons.first();
                                              
    // Simulate click on next button
    nextButton->click();
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
    
    // Disable automatic octave shifts when placing notes
    // We'll keep the code commented for reference
    /*
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
    */
    
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

MusicTrainer::music::Duration ScoreView::convertToMusicalDuration(double numericDuration)
{
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

void ScoreView::handleNoteAdded(int pitch, double duration, int position)
{
    if (!m_noteGrid || !m_viewportManager || !m_score) return;

    // Create event first to ensure consistent state
    MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(pitch);
    auto timeSignature = m_score->getTimeSignature();
    auto event = ::music::events::NoteAddedEvent::create(
        0, // voice index
        notePitch,
        convertToMusicalDuration(duration),
        timeSignature
    );
    
    // Get current grid dimensions
    auto dimensions = m_noteGrid->getDimensions();
    
    // Validate and clamp the position and pitch
    pitch = qBound(dimensions.minPitch, pitch, dimensions.maxPitch - 1);
    position = qBound(dimensions.startPosition, position, dimensions.endPosition - 1);

    // Apply event first to ensure model is updated
    event->apply(*m_score);
    
    // Rate limit only the visual update
    static QElapsedTimer lastNoteTime;
    if (lastNoteTime.isValid() && lastNoteTime.elapsed() < 50) {
        return; // Skip visual update but note is already added to model
    }
    lastNoteTime.restart();
    
    // Update visual representation
    MusicTrainer::music::Note note(notePitch, duration, position);
    m_noteGrid->addNote(note, 0, position);
    
    // Update only the affected region
    QRectF updateRect(
        position * GRID_UNIT - GRID_UNIT,
        pitch * NOTE_HEIGHT - NOTE_HEIGHT,
        GRID_UNIT * 2,
        NOTE_HEIGHT * 2
    );
    m_noteGrid->updateGrid(updateRect);
    
    // Check if we need to expand the viewport
    checkViewportExpansion();
}

void ScoreView::setupDurationToolbar()
{
    // Create toolbar and add it to a layout above the view
    m_durationToolbar = new QToolBar(this);
    m_durationToolbar->setMovable(false);
    
    // Create button group
    m_durationButtons = new QButtonGroup(this);
    m_durationButtons->setExclusive(true);
    
    // Duration label
    m_durationLabel = new QLabel(this);
    m_durationLabel->setMinimumWidth(80);
    m_durationToolbar->addWidget(m_durationLabel);
    
    // Create buttons for each duration
    struct DurationInfo {
        double value;
        QString name;
        QString icon;
    };
    
    std::vector<DurationInfo> durations = {
        {4.0, "Whole", ":/icons/whole"},
        {2.0, "Half", ":/icons/half"},
        {1.0, "Quarter", ":/icons/quarter"},
        {0.5, "Eighth", ":/icons/eighth"},
        {0.25, "16th", ":/icons/sixteenth"}
    };
    
    for (const auto& info : durations) {
        auto* button = new QPushButton(this);
        button->setCheckable(true);
        button->setToolTip(info.name + " note");
        button->setFixedSize(32, 32);
        // Set default style for now, later we can add icons
        button->setText(info.name.left(1));
        
        m_durationButtons->addButton(button);
        m_durationToolbar->addWidget(button);
        
        // Store duration value in button's data
        button->setProperty("duration", info.value);
        
        // Connect button to duration change
        connect(button, &QPushButton::clicked, this, [this, value = info.value]() {
            m_currentNoteDuration = value;
            updateDurationLabel();
            
            // Update note grid density based on the selected duration
            if (m_noteGrid) {
                m_noteGrid->setGridDensity(value);
            }
            
            // Update preview if currently selecting
            if (m_isSelecting) {
                QPointF musicalPos = mapToMusicalSpace(m_lastMousePos);
                auto dimensions = m_noteGrid->getDimensions();
                int position = qBound(dimensions.startPosition,
                                   static_cast<int>(qRound(musicalPos.x() * 4.0) / 4.0),
                                   dimensions.endPosition - 1);
                int pitch = qBound(dimensions.minPitch,
                                 qRound(musicalPos.y()),
                                 dimensions.maxPitch - 1);
                m_noteGrid->showNotePreview(position, pitch, m_currentNoteDuration);
            }
        });
    }
    
    // Set initial duration button
    if (auto buttons = m_durationButtons->buttons(); !buttons.isEmpty()) {
        for (auto* button : buttons) {
            if (button->property("duration").toDouble() == m_currentNoteDuration) {
                button->setChecked(true);
                break;
            }
        }
    }
    
    // Add toolbar to layout
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_durationToolbar);
    layout->addWidget(viewport());
    setLayout(layout);
    
    updateDurationLabel();
}

QString ScoreView::getDurationName(double duration) const
{
    if (duration == 4.0) return "Whole";
    if (duration == 2.0) return "Half";
    if (duration == 1.0) return "Quarter";
    if (duration == 0.5) return "Eighth";
    if (duration == 0.25) return "16th";
    return QString::number(duration) + " beats";
}

void ScoreView::updateDurationLabel()
{
    if (m_durationLabel) {
        m_durationLabel->setText(getDurationName(m_currentNoteDuration));
    }
}

} // namespace MusicTrainer::presentation