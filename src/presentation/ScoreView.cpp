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
    
    // Get available space accounting for toolbar
    QRect availableRect = viewport()->rect();
    if (m_durationToolbar) {
        availableRect.adjust(0, m_durationToolbar->height(), 0, 0);
    }
    
    if (m_viewportManager) {
        // Update viewport manager with new size
        m_viewportManager->updateViewSize(availableRect.size());
        
        // Recalculate scaling to fit available space
        auto dimensions = m_noteGrid->getDimensions();
        float labelMargin = 25.0f;
        float topMargin = 30.0f;
        float bottomMargin = 15.0f;
        
        int pitchRange = dimensions.maxPitch - dimensions.minPitch;
        
        // Calculate scaling factors based on available space
        float horizontalScale = availableRect.width() / (dimensions.endPosition * GRID_UNIT + 2 * labelMargin);
        float verticalScale = availableRect.height() / ((pitchRange * NOTE_HEIGHT) + topMargin + bottomMargin);
        
        // Use the smaller scale to ensure everything fits
        float scaleFactor = qMin(horizontalScale, verticalScale);
        scaleFactor = qMax(0.8f, qMin(scaleFactor, 1.5f));
        
        // Apply new scale
        QTransform transform;
        transform.scale(scaleFactor, scaleFactor);
        setTransform(transform);
        
        // Update viewport manager with new scale
        m_viewportManager->updateZoomLevel(scaleFactor);
        
        // Center the view on the content
        centerOn(QPointF(dimensions.endPosition * GRID_UNIT / 2.0,
                        (dimensions.minPitch + pitchRange / 2.0) * NOTE_HEIGHT));
                        
        // Update visuals
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
    if (!m_viewportManager || !m_noteGrid) {
        QGraphicsView::mousePressEvent(event);
        return;
    }
    
    // Convert screen coordinates to musical space
    QPointF musicalPoint = m_viewportManager->mapToMusicalSpace(event->pos(), this);
    int position = qFloor(musicalPoint.x());
    int pitch = qFloor(musicalPoint.y());
    
    // Check if clicked on an expansion button
    if (event->button() == Qt::LeftButton) {
        if (m_topExpandButton && m_topExpandButton->contains(m_topExpandButton->mapFromScene(mapToScene(event->pos())))) {
            expandGrid(GridDirection::Up, 12);
            event->accept();
            return;
        }
        if (m_bottomExpandButton && m_bottomExpandButton->contains(m_bottomExpandButton->mapFromScene(mapToScene(event->pos())))) {
            expandGrid(GridDirection::Down, 12);
            event->accept();
            return;
        }
        if (m_rightExpandButton && m_rightExpandButton->contains(m_rightExpandButton->mapFromScene(mapToScene(event->pos())))) {
            expandGrid(GridDirection::Right, 4);
            event->accept();
            return;
        }

        // If there's already a note at this position, start dragging it
        if (m_noteGrid->hasNoteAt(position, pitch)) {
            startNoteDrag(mapToScene(event->pos()));
            m_isDragging = true;
        } else {
            // Otherwise, start note placement
            m_isSelecting = true;
            m_lastMousePos = event->pos();
            m_noteGrid->showNotePreview(position, pitch, m_currentNoteDuration);
        }
        event->accept();
        return;
    }
    
    // Handle right-click for note deletion
    if (event->button() == Qt::RightButton) {
        deleteNoteAt(mapToScene(event->pos()));
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

void ScoreView::expandGrid(GridDirection direction, int amount)
{
    if (!m_viewportManager) return;
    
    // Store current viewport state
    auto currentBounds = m_viewportManager->getViewportBounds();
    
    // Expand grid in requested direction
    m_viewportManager->expandGrid(direction, amount);
    
    // Notify listeners about expansion
    emit viewportExpanded(m_viewportManager->getViewportBounds());
    
    // Update grid visuals
    updateGridVisuals();
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
    
    // Calculate available viewport height (accounting for duration toolbar)
    QRect viewportRect = viewport()->rect();
    int availableHeight = viewportRect.height();
    
    // Calculate scene rect to exactly match the current octave plus margins
    // Include additional space for expansion buttons and labels
    float topMargin = 30.0f;     // Space for measure numbers and top expansion button
    float bottomMargin = 15.0f;  // Space for bottom expansion button
    
    scene()->setSceneRect(
        -labelMargin,  // Space for labels
        dimensions.minPitch * NOTE_HEIGHT - topMargin,  // Space above for labels and buttons
        endPosition * GRID_UNIT + 2 * labelMargin,  // Grid width plus margins
        (pitchRange + 1) * NOTE_HEIGHT + topMargin + bottomMargin  // One octave plus space for buttons and labels
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
    
    // Calculate scale to fit the viewport, accounting for toolbar and margins
    float horizontalScale = viewportRect.width() / (endPosition * GRID_UNIT + 2 * labelMargin);
    float verticalScale = availableHeight / ((pitchRange * NOTE_HEIGHT) + topMargin + bottomMargin);
    
    // Use the smaller scale to ensure everything fits
    float scaleFactor = qMin(horizontalScale, verticalScale);
    
    // Limit the scale range for readability
    scaleFactor = qMax(0.8f, qMin(scaleFactor, 1.5f));
    
    // Apply scale
    scale(scaleFactor, scaleFactor);
    
    // Center on the grid, accounting for the top margin
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
    
    // Add measure numbers if enabled
    if (m_showMeasureNumbers) {
        // Clear any existing measure numbers
        QList<QGraphicsItem*> items = scene()->items();
        for (auto* item : items) {
            if (item->data(0).toString() == "measure_number") {
                scene()->removeItem(item);
                delete item;
            }
        }
        
        // Calculate visible measure range
        int startMeasure = static_cast<int>(musicalBounds.left()) / 4; // Assuming 4/4 time signature
        int endMeasure = static_cast<int>(musicalBounds.right()) / 4 + 1;
        
        // Add measure numbers
        for (int measure = startMeasure; measure <= endMeasure; ++measure) {
            if (measure < 0) continue;
            
            // Calculate position for measure number (start of measure)
            qreal xPos = measure * 4 * GRID_UNIT; // 4 beats per measure in 4/4
            qreal yPos = viewportSceneRect.top() - 20; // Above the staff
            
            // Create text item for measure number
            auto* textItem = new QGraphicsTextItem(QString::number(measure + 1)); // 1-based measure numbers
            textItem->setPos(xPos, yPos);
            textItem->setData(0, "measure_number"); // Tag for later identification
            scene()->addItem(textItem);
        }
    }
    
    // Add key signature if enabled
    if (m_showKeySignature && m_score) {
        // Clear any existing key signature
        QList<QGraphicsItem*> items = scene()->items();
        for (auto* item : items) {
            if (item->data(0).toString() == "key_signature") {
                scene()->removeItem(item);
                delete item;
            }
        }
        
        // Add key signature at the left side of the viewport
        qreal xPos = viewportSceneRect.left() + 10;
        qreal yPos = viewportSceneRect.top() + 10;
        
        // For now, just show a placeholder text
        // In a complete implementation, we would render actual key signature symbols
        auto* textItem = new QGraphicsTextItem("C Major"); // Placeholder
        textItem->setPos(xPos, yPos);
        textItem->setData(0, "key_signature"); // Tag for later identification
        scene()->addItem(textItem);
    }
    
    // Add voice labels if enabled
    if (m_showVoiceLabels && m_score) {
        // Clear any existing voice labels
        QList<QGraphicsItem*> items = scene()->items();
        for (auto* item : items) {
            if (item->data(0).toString() == "voice_label") {
                scene()->removeItem(item);
                delete item;
            }
        }
        
        // Add voice labels at the left side of the viewport
        for (size_t i = 0; i < m_score->getVoiceCount(); ++i) {
            if (auto* voice = m_score->getVoice(i)) {
                // Calculate position for voice label
                qreal xPos = viewportSceneRect.left() + 10;
                
                // Get the average pitch for this voice to position the label
                int avgPitch = dimensions.minPitch + (dimensions.maxPitch - dimensions.minPitch) / 2;
                if (!voice->getAllNotes().empty()) {  // Fixed: Use getAllNotes().empty() instead of getNoteCount()
                    // In a complete implementation, we would calculate the average pitch
                    // For now, just use a placeholder
                    avgPitch = 60 + static_cast<int>(i) * 12; // Middle C + offset for each voice
                }
                
                qreal yPos = avgPitch * NOTE_HEIGHT;
                
                // Create text item for voice label
                auto* textItem = new QGraphicsTextItem(QString("Voice %1").arg(i + 1));
                textItem->setPos(xPos, yPos);
                textItem->setData(0, "voice_label"); // Tag for later identification
                
                // Set color based on voice index
                QColor voiceColor;
                switch (i % 4) {
                    case 0: voiceColor = Qt::blue; break;
                    case 1: voiceColor = Qt::red; break;
                    case 2: voiceColor = Qt::green; break;
                    case 3: voiceColor = Qt::magenta; break;
                }
                textItem->setDefaultTextColor(voiceColor);
                
                scene()->addItem(textItem);
            }
        }
    }
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
    m_durationToolbar->setFixedHeight(40); // Set fixed height to ensure consistent layout
    
    // Create button group
    m_durationButtons = new QButtonGroup(this);
    m_durationButtons->setExclusive(true);
    
    // Duration label
    m_durationLabel = new QLabel(this);
    m_durationLabel->setMinimumWidth(80);
    m_durationLabel->setMaximumHeight(32);
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
    layout->setSpacing(0); // Minimize spacing between toolbar and viewport
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
void ScoreView::setShowMeasureNumbers(bool show)
{
    if (m_showMeasureNumbers == show) {
        return;
    }
    
    m_showMeasureNumbers = show;
    updateGridVisuals();
}

void ScoreView::setShowKeySignature(bool show)
{
    if (m_showKeySignature == show) {
        return;
    }
    
    m_showKeySignature = show;
    updateGridVisuals();
}

void ScoreView::setShowVoiceLabels(bool show)
{
    if (m_showVoiceLabels == show) {
        return;
    }
    
    m_showVoiceLabels = show;
    updateGridVisuals();
}

} // namespace MusicTrainer::presentation