#include "presentation/NoteGrid.h"
#include "presentation/GridCell.h"
#include "domain/music/Note.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include <QBrush>
#include <QPen>
#include <QDebug>

namespace MusicTrainer::presentation {

NoteGrid::NoteGrid(QGraphicsScene* scene)
    : m_scene(scene)
    , m_dimensions{60, 72, 0, 48}  // C4 to C5 (one octave), 12 measures (48 beats in 4/4 time)
    , m_noteCount(0)
{
    qDebug() << "NoteGrid::NoteGrid - Initial dimensions:" 
             << "Pitch range:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time range:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
    
    // Add initial gridlines with exact dimensions
    updateGridLines(QRectF(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        (m_dimensions.maxPitch - m_dimensions.minPitch) * NOTE_HEIGHT
    ), 1.0f);
}

NoteGrid::~NoteGrid() = default;

const NoteGrid::GridDimensions& NoteGrid::getDimensions() const
{
    return m_dimensions;
}

NoteGrid::GridDimensions NoteGrid::validateDimensions(const GridDimensions& dimensions) const 
{
    GridDimensions validated = dimensions;
    
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
    validated.startPosition = dimensions.startPosition;
    validated.endPosition = qMin(dimensions.startPosition + 48, dimensions.endPosition);
    
    return validated;
}

void NoteGrid::setDimensions(const GridDimensions& dimensions)
{
    // Store the previous dimensions for logging
    GridDimensions oldDimensions = m_dimensions;
    
    // Only allow octave change if within valid MIDI range
    if (dimensions.minPitch >= 0 && dimensions.minPitch + 12 <= 127) {
        m_dimensions.minPitch = dimensions.minPitch;
        m_dimensions.maxPitch = m_dimensions.minPitch + 12;  // Force exactly one octave
    }
    
    // Enforce 12 measures maximum
    m_dimensions.startPosition = dimensions.startPosition;
    m_dimensions.endPosition = qMin(dimensions.startPosition + 48,
                                  dimensions.endPosition);
    
    qDebug() << "NoteGrid::setDimensions - Old:" << oldDimensions.minPitch << "-" << oldDimensions.maxPitch
             << "New:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch;
    
    // Create scene rect based on constrained dimensions
    QRectF newBounds(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        12 * NOTE_HEIGHT  // Force one octave height
    );
    
    // Update visuals with constrained bounds
    updateGrid(newBounds);
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
    if (newMinPitch < 0 || newMinPitch + 12 > 127) {
        qDebug() << "NoteGrid::expandVertical - Blocked expansion outside MIDI range";
        return;
    }
    
    // Update pitch range while maintaining octave
    m_dimensions.minPitch = newMinPitch;
    m_dimensions.maxPitch = m_dimensions.minPitch + 12;
    
    // Create scene rect based on new dimensions
    QRectF newBounds(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        12 * NOTE_HEIGHT  // Force one octave height
    );
    
    // Update visuals
    updateGrid(newBounds);
}

void NoteGrid::expandHorizontal(int amount)
{
    // Block expansion beyond 12 measures (48 beats)
    if (m_dimensions.endPosition - m_dimensions.startPosition + amount > 48) {
        qDebug() << "NoteGrid::expandHorizontal - Blocked expansion beyond 12 measures";
        return;
    }
    
    m_dimensions.endPosition += amount;
    
    // Create scene rect based on new dimensions
    QRectF newBounds(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        12 * NOTE_HEIGHT  // Force one octave height
    );
    
    // Update visuals
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
    // Extract MIDI note number from the Pitch object
    int pitch = note.getPitch().getMidiNote(); // Get midi note number from Pitch object
    
    // Create a cell at the specified position
    auto cell = getOrCreateCell(position, pitch);
    
    // Set the note in the cell with proper duration and voice
    cell->setNote(note, voiceIndex);
    
    m_noteCount++;
}

void NoteGrid::updateGridLines(const QRectF& visibleRect, float zoomLevel)
{
    // Clear existing grid lines
    clearGridElements();
    
    // Convert scene coordinates to musical coordinates
    QRectF gridRect(
        visibleRect.x() / GRID_UNIT,
        visibleRect.y() / NOTE_HEIGHT,
        visibleRect.width() / GRID_UNIT,
        12  // Force one octave height
    );
    
    qDebug() << "NoteGrid::updateGridLines - Refresh -"
             << "SceneRect:" << visibleRect
             << "GridRect:" << gridRect
             << "Zoom:" << zoomLevel
             << "GridDims - Pitch:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
    
    // Create new grid lines within constrained bounds
    updateGridLineItems(true);  // Major lines
    updateGridLineItems(false); // Minor lines
}

void NoteGrid::updateGrid(const QRectF& bounds)
{
    // Ensure bounds maintain one octave height
    QRectF constrainedBounds = bounds;
    constrainedBounds.setHeight(12 * NOTE_HEIGHT);
    
    qDebug() << "NoteGrid::updateGrid - Original bounds:" << bounds
             << "Grid bounds:" << constrainedBounds
             << "NoteCount:" << m_noteCount
             << "ActiveCells:" << m_gridCells.size();
    
    // Update grid lines with constrained bounds
    updateGridLines(constrainedBounds, 1.0f);
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
        qDebug() << "NoteGrid::getOrCreateCell - Rejecting out-of-bounds cell at"
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
        m_noteCount++;
    }
    
    return cell.get();
}

void NoteGrid::updateGridLineItems(bool majorLines)
{
    // Implementation not needed for now
}

void NoteGrid::clearGridElements()
{
    // Helper function to clear a vector of grid items
    auto clearItems = [this](std::vector<QGraphicsItem*>& items) {
        for (auto item : items) {
            if (item) {
                m_scene->removeItem(item);
                delete item;
            }
        }
        items.clear();
    };
    
    // Clear all grid line vectors
    clearItems(m_horizontalLines);
    clearItems(m_verticalLines);
    clearItems(m_majorHorizontalLines);
    clearItems(m_majorVerticalLines);
}

} // namespace MusicTrainer::presentation