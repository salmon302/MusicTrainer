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
    
    // Enforce exactly one octave (12 semitones)
    validated.maxPitch = validated.minPitch + getOctaveRange();
    
    // Ensure pitch range stays within valid MIDI range (0-127)
    if (validated.minPitch < 0) {
        validated.minPitch = 0;
        validated.maxPitch = getOctaveRange();
    }
    else if (validated.maxPitch > 127) {
        validated.maxPitch = 127;
        validated.minPitch = validated.maxPitch - getOctaveRange();
    }
    
    // Enforce 12 measures maximum (48 beats in 4/4 time)
    validated.endPosition = qMin(validated.startPosition + 48, validated.endPosition);
    
    return validated;
}

void NoteGrid::setDimensions(const GridDimensions& dimensions)
{
    // Store the previous dimensions for logging
    GridDimensions oldDimensions = m_dimensions;
    GridDimensions validDimensions = validateDimensions(dimensions);
    
    // Update dimensions
    m_dimensions = validDimensions;
    
    qDebug() << "NoteGrid::setDimensions - Old:" << oldDimensions.minPitch << "-" << oldDimensions.maxPitch
             << "New:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch;
    
    // Create scene rect based on constrained dimensions
    updateGrid(QRectF(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        12 * NOTE_HEIGHT  // Force one octave height
    ));
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
             
    // Create scene rect for current state
    QRectF oldBounds(
        0,
        m_dimensions.minPitch * NOTE_HEIGHT,
        m_dimensions.endPosition * GRID_UNIT,
        12 * NOTE_HEIGHT
    );
    
    // Update pitch range while maintaining octave size
    m_dimensions.minPitch = newMinPitch;
    m_dimensions.maxPitch = newMaxPitch;
    
    // Create scene rect for new state
    QRectF newBounds(
        0,
        m_dimensions.minPitch * NOTE_HEIGHT,
        m_dimensions.endPosition * GRID_UNIT,
        12 * NOTE_HEIGHT
    );
    
    // Clear and redraw grid with expanded bounds
    clearGridElements();
    updateGridLines(newBounds, 1.0f);
    
    // Force scene update for the affected area
    m_scene->update(oldBounds.united(newBounds));
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
             
    m_dimensions.endPosition += amount;
    
    // Create scene rect based on new dimensions
    QRectF newBounds(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        12 * NOTE_HEIGHT  // Force one octave height
    );
    
    // Update visuals - reestablish the grid boundaries after expansion
    clearGridElements();  // Clear existing grid elements first
    updateGrid(newBounds);
}

void NoteGrid::updateFromScore(std::shared_ptr<MusicTrainer::music::Score> score)
{
    clear();
    if (!score) return;

    // Update grid's time signature from score
    m_dimensions.timeSignature = score->getTimeSignature();
    
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
    
    // Create a cell at the specified position - let getOrCreateCell handle bounds checking
    auto cell = getOrCreateCell(position, pitch);
    if (cell) {
        // Set the note in the cell with proper duration and voice
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
    
    // Clear and update grid elements with a single scene update
    m_scene->blockSignals(true);  // Block updates during changes
    clearGridElements();
    updateGridLines(validBounds, 1.0f);
    m_scene->blockSignals(false); // Re-enable updates
    
    // Force a full scene update
    m_scene->update(m_scene->sceneRect());
    
    // Request view update through the scene
    if (m_scene->views().size() > 0) {
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
    // Enforce grid bounds
    if (position < m_dimensions.startPosition || position >= m_dimensions.endPosition ||
        pitch < m_dimensions.minPitch || pitch >= m_dimensions.maxPitch) {
        qDebug() << "NoteGrid::getOrCreateCell - Clamping out-of-bounds cell at"
                 << "Position:" << position << "Pitch:" << pitch
                 << "Grid bounds:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
                 << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
        
        // Clamp to valid range
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

    // Draw horizontal pitch lines
    for (int pitch = m_dimensions.minPitch; pitch <= m_dimensions.maxPitch; pitch += horizontalStep) {
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

    // Draw vertical time division lines
    for (int pos = 0; pos <= m_dimensions.endPosition; pos += verticalStep) {
        auto* line = new QGraphicsLineItem(
            pos * GRID_UNIT,
            m_dimensions.minPitch * NOTE_HEIGHT,
            pos * GRID_UNIT,
            m_dimensions.maxPitch * NOTE_HEIGHT
        );
        line->setPen(linePen);
        m_scene->addItem(line);
        
        if (majorLines) {
            m_majorVerticalLines.push_back(line);
        } else {
            m_verticalLines.push_back(line);
        }
    }

    // Add expansion buttons for major lines only
    if (majorLines) {
        // Top expansion button - full width
        auto* topArrow = m_scene->addRect(
            0,
            (m_dimensions.minPitch - 1) * NOTE_HEIGHT,
            m_dimensions.endPosition * GRID_UNIT,
            NOTE_HEIGHT,
            QPen(Qt::black),
            QBrush(QColor(240, 240, 240))
        );
        topArrow->setZValue(10);
        m_majorHorizontalLines.push_back(topArrow);

        // Bottom expansion button - full width
        auto* bottomArrow = m_scene->addRect(
            0,
            m_dimensions.maxPitch * NOTE_HEIGHT,
            m_dimensions.endPosition * GRID_UNIT,
            NOTE_HEIGHT,
            QPen(Qt::black),
            QBrush(QColor(240, 240, 240))
        );
        bottomArrow->setZValue(10);
        m_majorHorizontalLines.push_back(bottomArrow);

        // Right expansion button - full height including expansion areas
        auto* rightArrow = m_scene->addRect(
            m_dimensions.endPosition * GRID_UNIT,
            (m_dimensions.minPitch - 1) * NOTE_HEIGHT,
            GRID_UNIT,
            (m_dimensions.maxPitch - m_dimensions.minPitch + 2) * NOTE_HEIGHT,
            QPen(Qt::black),
            QBrush(QColor(240, 240, 240))
        );
        rightArrow->setZValue(10);
        m_majorVerticalLines.push_back(rightArrow);
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