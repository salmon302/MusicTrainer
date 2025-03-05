#include "presentation/NoteGrid.h"
#include "presentation/GridCell.h"
#include "domain/music/Note.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <QGraphicsView> // Added include for QGraphicsView

namespace MusicTrainer::presentation {

NoteGrid::NoteGrid(QGraphicsScene* scene)
    : m_scene(scene)
    , m_dimensions{60, 72, 0, 48}  // C4 to C5 (one octave), 12 measures (48 beats in 4/4 time)
    , m_noteCount(0)
{
    qDebug() << "NoteGrid::NoteGrid - Initial dimensions:" 
             << "Pitch range:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time range:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
    
    // Create initial scene rect with one octave height
    QRectF initialBounds(
        0,  // Start at left edge
        m_dimensions.minPitch * NOTE_HEIGHT,  // Start at C4
        m_dimensions.endPosition * GRID_UNIT,  // Full width
        12 * NOTE_HEIGHT  // Exactly one octave
    );
    
    // Block signals during initial setup
    m_scene->blockSignals(true);
    
    // Set up initial grid lines
    updateGridLines(initialBounds, 1.0f);
    
    // Re-enable signals and force update
    m_scene->blockSignals(false);
    m_scene->update(m_scene->sceneRect());
    
    // Ensure view is updated
    if (m_scene->views().size() > 0) {
        m_scene->views().first()->viewport()->update();
    }
}

NoteGrid::~NoteGrid() {
    clearGridElements();
    delete m_previewIndicator;
}

const NoteGrid::GridDimensions& NoteGrid::getDimensions() const
{
    return m_dimensions;
}

NoteGrid::GridDimensions NoteGrid::validateDimensions(const GridDimensions& dimensions) const 
{
    GridDimensions validated = dimensions;
    
    // Ensure valid ranges
    validated.startPosition = qMax(0, validated.startPosition);
    validated.endPosition = qMax(validated.startPosition + 1, validated.endPosition);
    
    // Ensure minimum pitch range is at least one octave (12 semitones)
    if (validated.maxPitch - validated.minPitch < getMinimumOctaveRange()) {
        validated.maxPitch = validated.minPitch + getMinimumOctaveRange();
    }
    
    // Ensure pitch range stays within valid MIDI range (0-127)
    if (validated.minPitch < 0) {
        validated.minPitch = 0;
        validated.maxPitch = qMin(127, validated.minPitch + (validated.maxPitch - validated.minPitch));
    }
    else if (validated.maxPitch > 127) {
        validated.maxPitch = 127;
        validated.minPitch = qMax(0, validated.maxPitch - (validated.maxPitch - validated.minPitch));
    }
    
    // Enforce 12 measures maximum (48 beats in 4/4 time)
    validated.endPosition = qMin(validated.startPosition + 48, validated.endPosition);
    
    return validated;
}

void NoteGrid::setDimensions(const GridDimensions& dimensions)
{
    // Store the previous dimensions for comparison
    GridDimensions oldDimensions = m_dimensions;
    GridDimensions validDimensions = validateDimensions(dimensions);
    
    // Skip update if dimensions haven't changed
    if (oldDimensions.minPitch == validDimensions.minPitch &&
        oldDimensions.maxPitch == validDimensions.maxPitch &&
        oldDimensions.startPosition == validDimensions.startPosition &&
        oldDimensions.endPosition == validDimensions.endPosition) {
        return;
    }
    
    // Update dimensions
    m_dimensions = validDimensions;
    
    qDebug() << "NoteGrid::setDimensions - Old:" << oldDimensions.minPitch << "-" << oldDimensions.maxPitch
             << "New:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Range:" << (m_dimensions.maxPitch - m_dimensions.minPitch);
    
    // Hide notes that are now outside the visible range
    for (auto& posEntry : m_gridCells) {
        for (auto& pitchEntry : posEntry.second) {
            auto cell = pitchEntry.second.get();
            if (cell) {
                // Check if the note's pitch is outside the new range
                if (pitchEntry.first < m_dimensions.minPitch || 
                    pitchEntry.first >= m_dimensions.maxPitch) {
                    cell->clear();
                }
            }
        }
    }

    // Calculate new scene rect with margins for labels and expansion buttons
    float labelMargin = 25.0f;
    float pitchRangeHeight = (m_dimensions.maxPitch - m_dimensions.minPitch + 2) * NOTE_HEIGHT;
    
    QRectF newSceneRect(
        -labelMargin,  // Space for labels
        (m_dimensions.minPitch - 1) * NOTE_HEIGHT,  // Extra space for expansion button
        m_dimensions.endPosition * GRID_UNIT + 2 * labelMargin,  // Width plus margins
        pitchRangeHeight  // Full pitch range height plus space for buttons
    );

    // Block signals during scene rect update
    m_scene->blockSignals(true);
    
    // Update scene rect
    m_scene->setSceneRect(newSceneRect);
    
    // Create bounds for grid update that cover the full pitch range
    QRectF gridBounds(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        (m_dimensions.maxPitch - m_dimensions.minPitch) * NOTE_HEIGHT
    );
    
    // Update grid visuals
    updateGrid(gridBounds);
    
    // Re-enable signals and force update of the changed region
    m_scene->blockSignals(false);
    
    // Update the entire affected area
    m_scene->update(m_scene->sceneRect());
}

void NoteGrid::expandVertical(int minPitchDelta, int maxPitchDelta)
{
    // Block any expansion that would change the octave size
    if (abs(maxPitchDelta - minPitchDelta) != 0) {
        qDebug() << "NoteGrid::expandVertical - Blocked non-octave-preserving expansion";
        return;
    }
    
    // Check if new pitch range would be valid
    int newMinPitch = m_dimensions.minPitch + minPitchDelta;
    int newMaxPitch = m_dimensions.maxPitch + maxPitchDelta;
    
    if (newMinPitch < 0 || newMaxPitch > 127) {
        qDebug() << "NoteGrid::expandVertical - Blocked expansion outside MIDI range";
        return;
    }
    
    qDebug() << "NoteGrid::expandVertical - Shifting octave from" 
             << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "to" << newMinPitch << "-" << newMaxPitch;
             
    // Update dimensions
    m_dimensions.minPitch = newMinPitch;
    m_dimensions.maxPitch = newMaxPitch;
    
    // Force update of all grid elements
    if (m_scene) {
        QRectF updateRect = m_scene->sceneRect();
        updateGrid(updateRect);
        
        // Ensure scene knows it needs to redraw
        m_scene->update(m_scene->sceneRect());
    }
}

void NoteGrid::expandHorizontal(int amount)
{
    // Block expansion beyond 12 measures (48 beats)
    if (m_dimensions.endPosition - m_dimensions.startPosition + amount > 48) {
        qDebug() << "NoteGrid::expandHorizontal - Blocked expansion beyond 12 measures";
        return;
    }
    
    qDebug() << "NoteGrid::expandHorizontal - Expanding by" << amount 
             << "beats from" << m_dimensions.endPosition;
             
    // Store old bounds for update region
    QRectF oldBounds = m_scene->sceneRect();
    
    // Update dimensions
    m_dimensions.endPosition += amount;
    
    // Calculate new scene rect with margins for labels and expansion buttons
    float labelMargin = 25.0f;
    QRectF newSceneRect(
        -labelMargin,  // Space for labels
        (m_dimensions.minPitch - 1) * NOTE_HEIGHT,  // Extra space for expansion button
        m_dimensions.endPosition * GRID_UNIT + 2 * labelMargin,  // Width plus margins
        (14 * NOTE_HEIGHT)  // One octave plus space for expansion buttons
    );
    
    // Update scene bounds first
    m_scene->setSceneRect(newSceneRect);
    
    // Create bounds for grid update
    QRectF newBounds(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        12 * NOTE_HEIGHT  // Force one octave height
    );
    
    // Update visuals - reestablish the grid boundaries after expansion
    clearGridElements();  // Clear existing grid elements first
    updateGridLines(newBounds, 1.0f);
    
    // Force scene update for the affected area
    m_scene->update(oldBounds.united(newSceneRect));
}

void NoteGrid::updateFromScore(std::shared_ptr<MusicTrainer::music::Score> score)
{
    clear();
    if (!score) return;

    // Update grid's time signature from score - extract the numeric value from the TimeSignature struct
    auto timeSignature = score->getTimeSignature();
    // Calculate beats per measure based on time signature
    // For example, 4/4 would be 4.0 beats per measure
    m_dimensions.timeSignature = static_cast<float>(timeSignature.beats);
    
    // Get all voices from the score and their notes
    const auto& voices = score->getVoices();
    
    // Loop through each voice
    for (size_t voiceIndex = 0; voiceIndex < voices.size(); ++voiceIndex) {
        const auto& voice = voices[voiceIndex];
        if (!voice) continue;
        
        // Use getAllNotes() to get the vector of notes and its size
        const auto& notes = voice->getAllNotes();
        size_t noteCount = notes.size();
        
        // Add each note to the grid with its corresponding voice index
        int position = 0; // Start at position 0
        for (size_t i = 0; i < noteCount; ++i) {
            // Use getNoteAt instead of getNote
            const auto* vnote = voice->getNoteAt(i);
            if (!vnote) continue;
            
            // Use -> operator since vnote is a pointer
            int midiNote = vnote->getPitch().getMidiNote();
            double duration = vnote->getDuration();
            
            // Create a Note with correct constructor (pitch, duration, position)
            MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(midiNote);
            MusicTrainer::music::Note note(notePitch, duration, position);
            
            // Add the note to the grid
            addNote(note, voiceIndex, position);
            
            // Move position forward by note duration
            position += duration;
        }
    }
    
    // Update the grid with the new content
    updateGrid(QRectF(m_dimensions.startPosition, m_dimensions.minPitch,
                     m_dimensions.endPosition - m_dimensions.startPosition,
                     m_dimensions.maxPitch - m_dimensions.minPitch));
}

void NoteGrid::addNote(const MusicTrainer::music::Note& note, int voiceIndex)
{
    // Forward to the version that takes a position parameter with default position 0
    addNote(note, voiceIndex, 0);
}

void NoteGrid::addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position)
{
    // Get the raw MIDI note number from the pitch
    int pitch = note.getPitch().getMidiNote();
    
    qDebug() << "NoteGrid::addNote -"
             << "Position:" << position
             << "MIDI pitch:" << pitch
             << "Voice:" << voiceIndex;
    
    // Create a cell at the specified position without auto-expanding
    auto cell = getOrCreateCell(position, pitch);
    if (cell) {
        m_noteCount++;
        cell->setNote(note, voiceIndex);
    }
}

void NoteGrid::updateGridLines(const QRectF& visibleRect, float zoomLevel)
{
    // Convert bounds to integral musical coordinates and validate
    int startPos = qMax(0, qFloor(visibleRect.left() / GRID_UNIT));
    int endPos = m_dimensions.endPosition;
    int minPitch = m_dimensions.minPitch;
    int maxPitch = m_dimensions.maxPitch;
    
    // Clear existing grid elements
    clearGridElements();
    
    qDebug() << "NoteGrid::updateGridLines - Refresh -"
             << "SceneRect:" << visibleRect
             << "Musical coords - Time:" << startPos << "-" << endPos
             << "Pitch:" << minPitch << "-" << maxPitch
             << "Zoom:" << zoomLevel;
    
    // Draw grid lines within valid bounds
    updateGridLineItems(true);  // Major lines
    updateGridLineItems(false); // Minor lines
}

void NoteGrid::updateGrid(const QRectF& bounds)
{
    // Convert bounds to integral musical coordinates and validate
    int startPos = qMax(0, qFloor(bounds.left() / GRID_UNIT));
    int endPos = qMin(m_dimensions.endPosition, qCeil(bounds.right() / GRID_UNIT));
    int minPitch = qBound(0, qFloor(bounds.top() / NOTE_HEIGHT), 127);
    int maxPitch = qBound(0, qCeil(bounds.bottom() / NOTE_HEIGHT), 127);
    
    // Skip update if bounds haven't changed
    static QRectF lastBounds;
    if (lastBounds == bounds) {
        return;
    }
    lastBounds = bounds;
    
    // Construct valid bounds in scene coordinates
    QRectF validBounds(
        startPos * GRID_UNIT,
        minPitch * NOTE_HEIGHT,
        (endPos - startPos) * GRID_UNIT,
        (maxPitch - minPitch) * NOTE_HEIGHT
    );
    
    qDebug() << "NoteGrid::updateGrid - Original bounds:" << bounds
             << "Grid bounds:" << validBounds
             << "Musical coords - Time:" << startPos << "-" << endPos
             << "Pitch:" << minPitch << "-" << maxPitch;
    
    // Block updates while modifying scene
    m_scene->blockSignals(true);
    
    // Only clear and update grid elements if really needed
    static int lastStartPos = -1;
    static int lastEndPos = -1;
    static int lastMinPitch = -1;
    static int lastMaxPitch = -1;
    
    bool needsUpdate = lastStartPos != startPos || 
                      lastEndPos != endPos ||
                      lastMinPitch != minPitch ||
                      lastMaxPitch != maxPitch;
                      
    if (needsUpdate) {
        clearGridElements();
        updateGridLines(validBounds, 1.0f);
        
        lastStartPos = startPos;
        lastEndPos = endPos;
        lastMinPitch = minPitch;
        lastMaxPitch = maxPitch;
    }
    
    m_scene->blockSignals(false);
    
    // Only request scene update for the affected area
    m_scene->update(validBounds);
    
    // Only request scene update for the affected area
    m_scene->update(validBounds);
    
    // Request view update if needed
    if (needsUpdate && m_scene->views().size() > 0) {
        m_scene->views().first()->viewport()->update();
    }
}

void NoteGrid::clear()
{
    m_gridCells.clear();
    m_noteCount = 0;
    updateGridLines(QRectF(m_dimensions.startPosition, m_dimensions.minPitch,
                          m_dimensions.endPosition - m_dimensions.startPosition,
                          m_dimensions.maxPitch - m_dimensions.minPitch), 1.0f);
}

void NoteGrid::compactUnusedRegions(const QRectF& visibleRect, const QRectF& bufferZone)
{
    for (auto posIt = m_gridCells.begin(); posIt != m_gridCells.end();) {
        bool keepPosition = false;
        
        for (auto pitchIt = posIt->second.begin(); pitchIt != posIt->second.end();) {
            if (pitchIt->second && pitchIt->second->isVisible(bufferZone)) {
                keepPosition = true;
                ++pitchIt;
            } else {
                pitchIt = posIt->second.erase(pitchIt);
                m_noteCount--;
            }
        }
        
        if (!keepPosition) {
            posIt = m_gridCells.erase(posIt);
        } else {
            ++posIt;
        }
    }
}

bool NoteGrid::hasNoteAt(int position, int pitch) const
{
    auto posIt = m_gridCells.find(position);
    if (posIt == m_gridCells.end()) return false;
    
    auto pitchIt = posIt->second.find(pitch);
    return pitchIt != posIt->second.end() && 
           pitchIt->second && 
           pitchIt->second->isVisible(QRectF());
}

int NoteGrid::getNoteCount() const
{
    return m_noteCount;
}

GridCell* NoteGrid::getOrCreateCell(int position, int pitch)
{
    // Clamp position and pitch to current grid bounds
    if (position < m_dimensions.startPosition || position >= m_dimensions.endPosition ||
        pitch < m_dimensions.minPitch || pitch >= m_dimensions.maxPitch) {
        
        qDebug() << "NoteGrid::getOrCreateCell - Clamping out-of-bounds note at"
                 << "Position:" << position << "Pitch:" << pitch
                 << "Grid bounds:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
                 << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
        
        // Always clamp both position and pitch to current bounds
        position = qBound(m_dimensions.startPosition, position, m_dimensions.endPosition - 1);
        pitch = qBound(m_dimensions.minPitch, pitch, m_dimensions.maxPitch - 1);
    }
    
    // Create the cell if it doesn't exist
    auto& pitchMap = m_gridCells[position];
    auto& cell = pitchMap[pitch];
    if (!cell) {
        cell = std::make_unique<GridCell>(position, pitch, m_scene);
    }
    
    return cell.get();
}

void NoteGrid::updateGridLineItems(bool majorLines)
{
    // Set up line style based on major/minor lines
    QPen linePen(majorLines ? Qt::black : Qt::gray);
    linePen.setWidth(majorLines ? 2 : 1);

    // Calculate step sizes
    int horizontalStep = majorLines ? 12 : 1;  // One octave or one semitone
    int verticalStep = majorLines ? 4 : 1;     // One bar or one beat

    // Draw horizontal pitch lines across the entire range - support multi-octave ranges
    // Start at the lowest C below minPitch (for octave markings)
    int startPitch = m_dimensions.minPitch - (m_dimensions.minPitch % 12);
    
    for (int pitch = startPitch; pitch <= m_dimensions.maxPitch; pitch++) {
        // Draw major lines only at octave boundaries, minor lines for all semitones
        if (majorLines && pitch % 12 != 0) continue;
        if (!majorLines && pitch % 12 == 0) continue;  // Skip C notes for minor lines
        
        // Only draw if within the current range
        if (pitch >= m_dimensions.minPitch && pitch <= m_dimensions.maxPitch) {
            auto* line = new QGraphicsLineItem(
                0,  // Start from left edge
                pitch * NOTE_HEIGHT,
                m_dimensions.endPosition * GRID_UNIT,  // Extend to right edge
                pitch * NOTE_HEIGHT
            );
            line->setPen(linePen);
            m_scene->addItem(line);
            
            if (majorLines) {
                m_majorHorizontalLines.push_back(line);
                
                // Add octave labels (e.g., "C4") for C notes
                if (pitch % 12 == 0) {
                    int octave = (pitch / 12) - 1;
                    auto* label = new QGraphicsTextItem(QString("C%1").arg(octave));
                    label->setDefaultTextColor(Qt::black);
                    label->setPos(
                        -GRID_UNIT,  // Place left of grid
                        pitch * NOTE_HEIGHT - NOTE_HEIGHT/2  // Centered on pitch line
                    );
                    m_scene->addItem(label);
                    m_horizontalLines.push_back(label);
                }
            } else {
                m_horizontalLines.push_back(line);
            }
        }
    }

    // Draw vertical time division lines spanning across all octaves
    for (int pos = 0; pos <= m_dimensions.endPosition; pos += verticalStep) {
        // Skip minor lines at major boundaries
        if (!majorLines && pos % 4 == 0) continue;
        // Skip major lines at non-boundaries
        if (majorLines && pos % 4 != 0) continue;
        
        auto* line = new QGraphicsLineItem(
            pos * GRID_UNIT,
            m_dimensions.minPitch * NOTE_HEIGHT,  // Start at current min pitch
            pos * GRID_UNIT,
            m_dimensions.maxPitch * NOTE_HEIGHT   // End at current max pitch
        );
        line->setPen(linePen);
        m_scene->addItem(line);
        
        if (majorLines) {
            m_majorVerticalLines.push_back(line);
            
            // Add measure numbers for major lines
            if (pos % 4 == 0) {
                int measure = pos / 4 + 1;
                auto* label = new QGraphicsTextItem(QString::number(measure));
                label->setDefaultTextColor(Qt::black);
                label->setPos(
                    pos * GRID_UNIT + 2,  // Just right of the line
                    (m_dimensions.minPitch - 1) * NOTE_HEIGHT  // Just above the grid
                );
                m_scene->addItem(label);
                m_verticalLines.push_back(label);
            }
        } else {
            m_verticalLines.push_back(line);
        }
    }

    // Add expansion/collapse buttons only when at major boundaries
    if (majorLines) {
        bool isMultiOctave = (m_dimensions.maxPitch - m_dimensions.minPitch > 12);

        // Top expansion/collapse button (if not at top of MIDI range)
        if (m_dimensions.minPitch > 0) {
            QColor buttonColor = isMultiOctave ? QColor(220, 220, 255) : QColor(240, 240, 240);
            QString tooltip = isMultiOctave ? "Right-click to collapse octave" : "Click to expand octave up";
            
            auto* topArrow = new QGraphicsRectItem(
                0,
                (m_dimensions.minPitch - 1) * NOTE_HEIGHT,
                m_dimensions.endPosition * GRID_UNIT,
                NOTE_HEIGHT
            );
            topArrow->setPen(QPen(Qt::black));
            topArrow->setBrush(QBrush(buttonColor));
            topArrow->setToolTip(tooltip);
            topArrow->setZValue(10);
            m_majorHorizontalLines.push_back(topArrow);
            
            // Add up arrow or collapse indicator
            QString indicator = isMultiOctave ? "▼" : "▲";
            auto* arrowText = new QGraphicsTextItem(indicator);
            arrowText->setDefaultTextColor(Qt::black);
            arrowText->setPos(
                m_dimensions.endPosition * GRID_UNIT / 2,
                (m_dimensions.minPitch - 1) * NOTE_HEIGHT
            );
            m_scene->addItem(arrowText);
            m_majorHorizontalLines.push_back(arrowText);
        }

        // Bottom expansion/collapse button (if not at bottom of MIDI range)
        if (m_dimensions.maxPitch < 127) {
            QColor buttonColor = isMultiOctave ? QColor(220, 220, 255) : QColor(240, 240, 240);
            QString tooltip = isMultiOctave ? "Right-click to collapse octave" : "Click to expand octave down";
            
            auto* bottomArrow = new QGraphicsRectItem(
                0,
                m_dimensions.maxPitch * NOTE_HEIGHT,
                m_dimensions.endPosition * GRID_UNIT,
                NOTE_HEIGHT
            );
            bottomArrow->setPen(QPen(Qt::black));
            bottomArrow->setBrush(QBrush(buttonColor));
            bottomArrow->setToolTip(tooltip);
            bottomArrow->setZValue(10);
            m_majorHorizontalLines.push_back(bottomArrow);
            
            // Add down arrow or collapse indicator
            QString indicator = isMultiOctave ? "▲" : "▼";
            auto* arrowText = new QGraphicsTextItem(indicator);
            arrowText->setDefaultTextColor(Qt::black);
            arrowText->setPos(
                m_dimensions.endPosition * GRID_UNIT / 2,
                m_dimensions.maxPitch * NOTE_HEIGHT
            );
            m_scene->addItem(arrowText);
            m_majorHorizontalLines.push_back(arrowText);
        }

        // Right expansion/collapse button - extend across the full expanded range
        bool canCollapseHorizontal = m_dimensions.endPosition > 16;
        QColor rightButtonColor = canCollapseHorizontal ? QColor(220, 220, 255) : QColor(240, 240, 240);
        QString rightTooltip = canCollapseHorizontal ? "Right-click to collapse measures" : "Click to add measures";
        
        auto* rightArrow = new QGraphicsRectItem(
            m_dimensions.endPosition * GRID_UNIT,
            m_dimensions.minPitch * NOTE_HEIGHT,
            GRID_UNIT,
            (m_dimensions.maxPitch - m_dimensions.minPitch) * NOTE_HEIGHT
        );
        rightArrow->setPen(QPen(Qt::black));
        rightArrow->setBrush(QBrush(rightButtonColor));
        rightArrow->setToolTip(rightTooltip);
        rightArrow->setZValue(10);
        m_majorVerticalLines.push_back(rightArrow);
        
        // Add right arrow or collapse indicator
        QString rightIndicator = canCollapseHorizontal ? "◄" : "►";
        auto* rightArrowText = new QGraphicsTextItem(rightIndicator);
        rightArrowText->setDefaultTextColor(Qt::black);
        rightArrowText->setPos(
            m_dimensions.endPosition * GRID_UNIT,
            (m_dimensions.minPitch + (m_dimensions.maxPitch - m_dimensions.minPitch) / 2) * NOTE_HEIGHT
        );
        m_scene->addItem(rightArrowText);
        m_majorVerticalLines.push_back(rightArrowText);
    }
}

void NoteGrid::clearGridElements()
{
    // Remove only grid lines and labels while preserving note cells
    for (auto* item : m_horizontalLines) {
        m_scene->removeItem(item);
        delete item;
    }
    for (auto* item : m_verticalLines) {
        m_scene->removeItem(item);
        delete item;
    }
    for (auto* item : m_majorHorizontalLines) {
        m_scene->removeItem(item);
        delete item;
    }
    for (auto* item : m_majorVerticalLines) {
        m_scene->removeItem(item);
        delete item;
    }

    m_horizontalLines.clear();
    m_verticalLines.clear();
    m_majorHorizontalLines.clear();
    m_majorVerticalLines.clear();
}

void NoteGrid::showNotePreview(int position, int pitch) {
    if (!m_scene) return;
    
    // Create preview indicator if it doesn't exist
    if (!m_previewIndicator) {
        auto indicator = new QGraphicsRectItem();
        indicator->setBrush(QBrush(QColor(100, 100, 255, 128))); // Semi-transparent blue
        indicator->setPen(QPen(QColor(50, 50, 200), 1)); // Darker blue outline
        m_previewIndicator = indicator;
        m_scene->addItem(m_previewIndicator);
    }
    
    updatePreviewPosition(position, pitch);
    m_previewIndicator->show();
}

void NoteGrid::hideNotePreview() {
    if (m_previewIndicator) {
        m_previewIndicator->hide();
    }
}

void NoteGrid::updatePreviewPosition(int position, int pitch) {
    if (!m_previewIndicator) return;
    
    // Convert musical coordinates to scene coordinates
    qreal x = position * GRID_UNIT;
    qreal y = pitch * NOTE_HEIGHT;
    
    // Set the preview rectangle
    auto rect = static_cast<QGraphicsRectItem*>(m_previewIndicator);
    rect->setRect(x, y, GRID_UNIT, NOTE_HEIGHT);
}

} // namespace MusicTrainer::presentation