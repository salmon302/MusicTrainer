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
    , m_dimensions{48, 96, 0, 32} // C3 to C7, 32 beats
{
    qDebug() << "NoteGrid::NoteGrid - Initial dimensions:" 
             << "Pitch range:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time range:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
    updateGridLines(QRectF(0, m_dimensions.minPitch, 32, 48), 1.0f);
}

NoteGrid::~NoteGrid() = default;

const NoteGrid::GridDimensions& NoteGrid::getDimensions() const
{
    return m_dimensions;
}

void NoteGrid::setDimensions(const GridDimensions& dimensions)
{
    qDebug() << "NoteGrid::setDimensions - Old:" 
             << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << m_dimensions.startPosition << "-" << m_dimensions.endPosition
             << "New:" 
             << dimensions.minPitch << "-" << dimensions.maxPitch
             << dimensions.startPosition << "-" << dimensions.endPosition;
             
    m_dimensions = dimensions;
    updateGrid(QRectF(m_dimensions.startPosition, m_dimensions.minPitch,
                     m_dimensions.endPosition - m_dimensions.startPosition,
                     m_dimensions.maxPitch - m_dimensions.minPitch));
}

void NoteGrid::expandVertical(int minPitchDelta, int maxPitchDelta)
{
    m_dimensions.minPitch += minPitchDelta;
    m_dimensions.maxPitch += maxPitchDelta;
    updateGrid(QRectF(m_dimensions.startPosition, m_dimensions.minPitch,
                     m_dimensions.endPosition - m_dimensions.startPosition,
                     m_dimensions.maxPitch - m_dimensions.minPitch));
}

void NoteGrid::expandHorizontal(int amount)
{
    m_dimensions.endPosition += amount;
    updateGrid(QRectF(m_dimensions.startPosition, m_dimensions.minPitch,
                     m_dimensions.endPosition - m_dimensions.startPosition,
                     m_dimensions.maxPitch - m_dimensions.minPitch));
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
    qDebug() << "NoteGrid::updateGridLines -"
             << "VisibleRect:" << visibleRect
             << "Zoom:" << zoomLevel
             << "GridDims - Pitch:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition
             << "TimeSignature:" << m_dimensions.timeSignature.beats << "/" << m_dimensions.timeSignature.beatUnit.getTotalBeats();

    // Clear existing grid lines
    for (auto item : m_horizontalLines) {
        m_scene->removeItem(item);
        delete item;
    }
    m_horizontalLines.clear();

    for (auto item : m_verticalLines) {
        m_scene->removeItem(item);
        delete item;
    }
    m_verticalLines.clear();

    // Limit grid lines to visible area with a margin to prevent excessive line creation
    int minPitch = qMax(m_dimensions.minPitch, qFloor(visibleRect.top() / NOTE_HEIGHT) - 1);
    int maxPitch = qMin(m_dimensions.maxPitch, qCeil(visibleRect.bottom() / NOTE_HEIGHT) + 1);
    
    int startPos = qMax(m_dimensions.startPosition, qFloor(visibleRect.left() / GRID_UNIT) - 1);
    int endPos = qMin(m_dimensions.endPosition, qCeil(visibleRect.right() / GRID_UNIT) + 1);

    // Get score time signature beats per measure
    const auto& timeSignature = m_dimensions.timeSignature;
    int beatsPerMeasure = timeSignature.beats;

    // Create pens with different styles and thicknesses
    QPen beatPen(QColor(200, 200, 200), 0.5);        // Light gray for beat lines
    QPen measurePen(QColor(120, 120, 120), 1.0);     // Darker gray for measure lines
    QPen strongBeatPen(QColor(100, 100, 100), 0.8);  // Medium gray for strong beats
    QPen pitchClassPen(QColor(180, 180, 200), 0.7);  // Bluish for note pitch classes
    QPen octavePen(QColor(80, 80, 100), 1.2);        // Dark blue-gray for octave lines

    // Scale pen widths based on zoom level
    if (zoomLevel != 1.0f) {
        float penAdjust = 1.0f / zoomLevel;
        beatPen.setWidthF(0.5f * penAdjust);
        measurePen.setWidthF(1.0f * penAdjust);
        strongBeatPen.setWidthF(0.8f * penAdjust);
        pitchClassPen.setWidthF(0.7f * penAdjust);
        octavePen.setWidthF(1.2f * penAdjust);
    }

    // Add vertical gradient effect to horizontal lines
    QLinearGradient horizontalGradient(0, 0, 0, 1);
    horizontalGradient.setColorAt(0, QColor(240, 240, 245));
    horizontalGradient.setColorAt(1, QColor(230, 230, 235));

    // Horizontal lines (pitches) - only draw what's needed for the visible area
    for (int pitch = minPitch; pitch <= maxPitch; ++pitch) {
        float y = pitch * NOTE_HEIGHT;
        QPen* pen;
        
        if (pitch % 12 == 0) {           // C notes (octave lines)
            pen = &octavePen;
        } else if (pitch % 12 == 5 || pitch % 12 == 7) {  // F and G lines
            pen = &pitchClassPen;
        } else {
            pen = &beatPen;
        }
        
        auto line = m_scene->addLine(
            visibleRect.left(), 
            y,
            visibleRect.right(), 
            y,
            *pen
        );
        
        // Apply gradient background to octave and pitch class lines
        if (pitch % 12 == 0 || pitch % 12 == 5 || pitch % 12 == 7) {
            line->setZValue(-1); // Put background lines behind
            QGraphicsRectItem* bg = m_scene->addRect(
                visibleRect.left(),
                y - 0.5,
                visibleRect.right() - visibleRect.left(),
                1.0,
                Qt::NoPen,
                horizontalGradient
            );
            bg->setZValue(-2); // Put gradient behind lines
            m_horizontalLines.push_back(bg);
        }
        
        m_horizontalLines.push_back(line);
    }

    // Vertical lines (beats and measures) - only draw what's needed for the visible area
    for (int pos = startPos; pos <= endPos; ++pos) {
        float x = pos * GRID_UNIT;
        QPen* pen;
        
        bool isMeasureLine = pos % beatsPerMeasure == 0;
        bool isStrongBeat = pos % beatsPerMeasure == timeSignature.beats / 2;  // Middle of measure
        
        if (isMeasureLine) {          // Measure lines
            pen = &measurePen;
        } else if (isStrongBeat) {    // Strong beat lines
            pen = &strongBeatPen;
        } else {                      // Regular beat lines
            pen = &beatPen;
        }
        
        auto line = m_scene->addLine(
            x,
            visibleRect.top(),
            x,
            visibleRect.bottom(),
            *pen
        );

        // Add measure number labels only for visible measures
        if (isMeasureLine) {
            auto text = m_scene->addText(QString::number(pos / beatsPerMeasure + 1));
            text->setPos(x + 0.1, visibleRect.top());
            text->setDefaultTextColor(QColor(80, 80, 80));
            text->setScale(0.03 / zoomLevel);  // Scale text size with zoom
            m_verticalLines.push_back(text);
        }
        
        m_verticalLines.push_back(line);
    }
}

void NoteGrid::updateGrid(const QRectF& bounds)
{
    qDebug() << "NoteGrid::updateGrid -"
             << "Bounds:" << bounds
             << "NoteCount:" << m_noteCount
             << "ActiveCells:" << m_gridCells.size();

    // Update all visible cells
    for (auto& [pos, pitchMap] : m_gridCells) {
        for (auto& [pitch, cell] : pitchMap) {
            if (cell && cell->isVisible(bounds)) {
                cell->update(bounds, 1.0f);
            }
        }
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
    auto& pitchMap = m_gridCells[position];
    auto& cellPtr = pitchMap[pitch];
    if (!cellPtr) {
        qDebug() << "NoteGrid::getOrCreateCell - Creating new cell at"
                 << "Position:" << position
                 << "Pitch:" << pitch;
        cellPtr = std::make_unique<GridCell>(position, pitch, m_scene);
    }
    return cellPtr.get();
}

void NoteGrid::updateGridLineItems(bool majorLines)
{
    // Implementation not needed for now
}

} // namespace MusicTrainer::presentation