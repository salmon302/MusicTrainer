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
    , m_dimensions{60, 72, 0, 48} // C4 to C5 (1 octave), 48 beats (12 measures in 4/4 time)
{
    qDebug() << "NoteGrid::NoteGrid - Initial dimensions:" 
             << "Pitch range:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time range:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
    
    // Initialize grid lines with proper dimensions
    updateGridLines(QRectF(
        m_dimensions.startPosition, 
        m_dimensions.minPitch, 
        m_dimensions.endPosition - m_dimensions.startPosition, 
        m_dimensions.maxPitch - m_dimensions.minPitch
    ), 1.0f);
}

NoteGrid::~NoteGrid() = default;

const NoteGrid::GridDimensions& NoteGrid::getDimensions() const
{
    return m_dimensions;
}

void NoteGrid::setDimensions(const GridDimensions& dimensions)
{
    // Validate pitch range
    if (dimensions.minPitch > dimensions.maxPitch) {
        qWarning() << "NoteGrid::setDimensions - Invalid pitch range:" 
                   << dimensions.minPitch << "-" << dimensions.maxPitch;
        return;
    }
    
    // Validate position range
    if (dimensions.startPosition > dimensions.endPosition) {
        qWarning() << "NoteGrid::setDimensions - Invalid position range:"
                   << dimensions.startPosition << "-" << dimensions.endPosition;
        return;
    }
    
    // Store new dimensions
    m_dimensions = dimensions;
    
    // Ensure grid gets redrawn with new dimensions
    updateGridLines(m_scene->sceneRect(), 1.0);
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
    // Convert visible rect from scene coordinates to grid coordinates
    QRectF gridVisibleRect(
        visibleRect.left() / GRID_UNIT,
        visibleRect.top() / NOTE_HEIGHT,
        visibleRect.width() / GRID_UNIT,
        visibleRect.height() / NOTE_HEIGHT
    );
    
    qDebug() << "NoteGrid::updateGridLines - Refresh -"
             << "SceneRect:" << visibleRect
             << "GridRect:" << gridVisibleRect
             << "Zoom:" << zoomLevel
             << "GridDims - Pitch:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
             << "Time:" << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
    
    // Clear existing grid elements
    clearGridElements();
    
    // Constrain drawing range to actual grid dimensions plus a small buffer
    int minPitch = qMax(m_dimensions.minPitch - 12, qFloor(gridVisibleRect.top()));
    int maxPitch = qMin(m_dimensions.maxPitch + 12, qCeil(gridVisibleRect.bottom()));
    
    int startPos = qMax(m_dimensions.startPosition, qFloor(gridVisibleRect.left()));
    int endPos = qMin(m_dimensions.endPosition, qCeil(gridVisibleRect.right()));
    
    // Calculate beat divisions
    const auto& timeSignature = m_dimensions.timeSignature;
    int beatsPerMeasure = timeSignature.beats;
    
    // Create pens with appropriate styles
    QPen beatPen(QColor(220, 220, 220), 1.0);        // Light gray for beats
    QPen strongBeatPen(QColor(180, 180, 180), 1.5);  // Medium gray for strong beats
    QPen measurePen(QColor(100, 100, 100), 2.0);     // Dark gray for measures
    QPen pitchClassPen(QColor(200, 200, 220), 1.2);  // Light blue for pitch markers
    QPen octavePen(QColor(80, 80, 150), 2.0);        // Dark blue for octave lines
    
    // Scale pen widths based on zoom
    if (zoomLevel != 1.0f) {
        float penAdjust = 1.0f / qMax(0.1f, zoomLevel);
        beatPen.setWidthF(1.0f * penAdjust);
        strongBeatPen.setWidthF(1.5f * penAdjust);
        measurePen.setWidthF(2.0f * penAdjust);
        pitchClassPen.setWidthF(1.2f * penAdjust);
        octavePen.setWidthF(2.0f * penAdjust);
    }
    
    // Create white background
    QGraphicsRectItem* background = m_scene->addRect(
        m_dimensions.startPosition * GRID_UNIT,
        m_dimensions.minPitch * NOTE_HEIGHT,
        (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
        (m_dimensions.maxPitch - m_dimensions.minPitch) * NOTE_HEIGHT,
        Qt::NoPen,
        QBrush(QColor(252, 252, 255))
    );
    background->setZValue(-10);
    m_horizontalLines.push_back(background);
    
    // Draw piano key backgrounds
    for (int pitch = minPitch; pitch <= maxPitch; ++pitch) {
        float y = pitch * NOTE_HEIGHT;
        int pitchClass = pitch % 12;
        bool isBlackKey = (pitchClass == 1 || pitchClass == 3 || pitchClass == 6 || 
                          pitchClass == 8 || pitchClass == 10);
        
        QColor keyColor = isBlackKey ? QColor(242, 242, 245) : QColor(252, 252, 255);
        
        QGraphicsRectItem* keyBackground = m_scene->addRect(
            m_dimensions.startPosition * GRID_UNIT,
            y,
            (m_dimensions.endPosition - m_dimensions.startPosition) * GRID_UNIT,
            NOTE_HEIGHT,
            Qt::NoPen,
            QBrush(keyColor)
        );
        keyBackground->setZValue(-8);
        m_horizontalLines.push_back(keyBackground);
    }
    
    // Draw measure backgrounds
    for (int pos = startPos; pos <= endPos; pos += beatsPerMeasure) {
        bool isEvenMeasure = (pos / beatsPerMeasure) % 2 == 0;
        QColor measureColor = isEvenMeasure ? 
            QColor(255, 255, 255, 0) :    // Transparent
            QColor(248, 248, 255, 40);    // Very light blue tint
            
        float x = pos * GRID_UNIT;
        QGraphicsRectItem* measureBg = m_scene->addRect(
            x,
            minPitch * NOTE_HEIGHT,
            beatsPerMeasure * GRID_UNIT,
            (maxPitch - minPitch) * NOTE_HEIGHT,
            Qt::NoPen,
            measureColor
        );
        measureBg->setZValue(-5);
        m_majorVerticalLines.push_back(measureBg);
    }
    
    // Draw horizontal pitch lines
    for (int pitch = minPitch; pitch <= maxPitch; ++pitch) {
        float y = pitch * NOTE_HEIGHT;
        int pitchClass = pitch % 12;
        
        // Draw octave lines (C notes)
        if (pitchClass == 0) {
            auto line = m_scene->addLine(
                startPos * GRID_UNIT,
                y,
                endPos * GRID_UNIT,
                y,
                octavePen
            );
            line->setZValue(1);
            m_majorHorizontalLines.push_back(line);
            
            // Add octave label
            int octaveNumber = (pitch / 12) - 1;  // Standard octave numbering
            auto label = m_scene->addText(QString("C%1").arg(octaveNumber));
            label->setPos(startPos * GRID_UNIT - 25, y - 10);
            label->setDefaultTextColor(QColor(80, 80, 150));
            label->setScale(0.7 / zoomLevel);
            label->setZValue(2);
            m_majorHorizontalLines.push_back(label);
        }
        // Draw F note lines
        else if (pitchClass == 5) {
            auto line = m_scene->addLine(
                startPos * GRID_UNIT,
                y,
                endPos * GRID_UNIT,
                y,
                pitchClassPen
            );
            line->setZValue(-2);
            m_horizontalLines.push_back(line);
        }
    }
    
    // Draw vertical beat lines
    for (int pos = startPos; pos <= endPos; ++pos) {
        float x = pos * GRID_UNIT;
        
        bool isMeasure = pos % beatsPerMeasure == 0;
        bool isStrongBeat = (pos % beatsPerMeasure == beatsPerMeasure / 2) && 
                           (beatsPerMeasure % 2 == 0);
        
        QPen* pen = isMeasure ? &measurePen : 
                    isStrongBeat ? &strongBeatPen : &beatPen;
        
        auto line = m_scene->addLine(
            x,
            minPitch * NOTE_HEIGHT,
            x,
            maxPitch * NOTE_HEIGHT,
            *pen
        );
        
        line->setZValue(isMeasure ? 0 : -3);
        
        if (isMeasure) {
            m_majorVerticalLines.push_back(line);
            
            // Add measure number
            if (pos >= m_dimensions.startPosition) {
                int measureNumber = (pos / beatsPerMeasure) + 1;
                auto text = m_scene->addText(QString::number(measureNumber));
                text->setPos(x + 2, minPitch * NOTE_HEIGHT - 20);
                text->setDefaultTextColor(QColor(60, 60, 60));
                text->setScale(0.8 / zoomLevel);
                text->setZValue(5);
                m_majorVerticalLines.push_back(text);
            }
            
            // Add time signature at start
            if (pos == m_dimensions.startPosition) {
                QString timeSig = QString("%1/%2").arg(beatsPerMeasure)
                    .arg(static_cast<int>(4.0 / timeSignature.beatUnit.getTotalBeats()));
                auto tsText = m_scene->addText(timeSig);
                tsText->setPos(x, minPitch * NOTE_HEIGHT - 35);
                tsText->setDefaultTextColor(QColor(0, 0, 180));
                tsText->setScale(1.0 / zoomLevel);
                tsText->setZValue(5);
                m_majorVerticalLines.push_back(tsText);
            }
        } else {
            m_verticalLines.push_back(line);
        }
    }
}

void NoteGrid::updateGrid(const QRectF& bounds)
{
    // Convert input bounds from scene coordinates to grid coordinates if they are scene coordinates
    // (We detect this by checking if the values are much larger than typical grid coordinates)
    QRectF gridBounds = bounds;
    if (bounds.width() > 100 || bounds.height() > 100) {
        gridBounds = QRectF(
            bounds.left() / GRID_UNIT,
            bounds.top() / NOTE_HEIGHT,
            bounds.width() / GRID_UNIT,
            bounds.height() / NOTE_HEIGHT
        );
    }
    
    // Constrain to grid dimensions
    gridBounds = gridBounds.intersected(QRectF(
        m_dimensions.startPosition,
        m_dimensions.minPitch,
        m_dimensions.endPosition - m_dimensions.startPosition,
        m_dimensions.maxPitch - m_dimensions.minPitch
    ));
    
    qDebug() << "NoteGrid::updateGrid -"
             << "Original bounds:" << bounds
             << "Grid bounds:" << gridBounds
             << "NoteCount:" << m_noteCount
             << "ActiveCells:" << m_gridCells.size();

    // Update all visible cells within the constrained bounds
    float left = gridBounds.left();
    float right = gridBounds.right();
    float top = gridBounds.top();
    float bottom = gridBounds.bottom();
    
    for (auto& [pos, pitchMap] : m_gridCells) {
        // Only process if the position is within bounds
        if (pos >= left - 1 && pos <= right + 1) {
            for (auto& [pitch, cell] : pitchMap) {
                if (pitch >= top - 1 && pitch <= bottom + 1 && cell) {
                    cell->update(gridBounds, 1.0f);
                }
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
    // Prevent notes outside of the defined grid area
    if (pitch < m_dimensions.minPitch || pitch > m_dimensions.maxPitch || 
        position < m_dimensions.startPosition || position > m_dimensions.endPosition) {
        qDebug() << "NoteGrid::getOrCreateCell - Rejecting out-of-bounds cell at"
                 << "Position:" << position
                 << "Pitch:" << pitch
                 << "Grid bounds:" << m_dimensions.minPitch << "-" << m_dimensions.maxPitch
                 << m_dimensions.startPosition << "-" << m_dimensions.endPosition;
        position = qBound(m_dimensions.startPosition, position, m_dimensions.endPosition);
        pitch = qBound(m_dimensions.minPitch, pitch, m_dimensions.maxPitch);
    }

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