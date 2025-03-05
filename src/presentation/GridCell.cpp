#include "presentation/GridCell.h"
#include "domain/music/Note.h"
#include <QPainter>
#include <QDebug>

namespace MusicTrainer::presentation {

GridCell::GridCell(int position, int pitch, QGraphicsScene* scene)
    : m_position(position)
    , m_pitch(pitch)
    , m_scene(scene)
    , m_noteRect(nullptr)
    , m_voiceIndex(-1)
{
    // Initialize the note rectangle
    m_noteRect = m_scene->addRect(
        position * GRID_UNIT,
        pitch * NOTE_HEIGHT,
        GRID_UNIT,
        NOTE_HEIGHT,
        QPen(Qt::NoPen),
        QBrush(Qt::transparent)
    );
    m_noteRect->setVisible(false);
}

GridCell::~GridCell()
{
    if (m_noteRect) {
        m_scene->removeItem(m_noteRect);
        delete m_noteRect;
    }
}

bool GridCell::isVisible(const QRectF& viewport) const
{
    if (!m_noteRect) return false;
    
    // Check if this cell's pitch is within the allowed range
    QPointF scenePos = m_noteRect->scenePos();
    float pitchInScene = scenePos.y() / NOTE_HEIGHT;
    
    // If viewport is empty, only check if note exists
    if (viewport.isNull()) {
        return m_note.has_value() && m_noteRect->isVisible();
    }
    
    // Check if pitch is within viewport bounds
    if (pitchInScene < viewport.top() / NOTE_HEIGHT || 
        pitchInScene >= viewport.bottom() / NOTE_HEIGHT) {
        return false;
    }
    
    QRectF cellRect = m_noteRect->boundingRect().translated(scenePos);
    return viewport.intersects(cellRect) && m_note.has_value() && m_noteRect->isVisible();
}

void GridCell::setNote(const MusicTrainer::music::Note& note, int voiceIndex)
{
    m_note = note;
    m_voiceIndex = voiceIndex;
    
    // Make sure rectangle exists and is sized correctly
    if (!m_noteRect) {
        m_noteRect = m_scene->addRect(
            m_position * GRID_UNIT,
            m_pitch * NOTE_HEIGHT,
            GRID_UNIT,
            NOTE_HEIGHT,
            QPen(Qt::black),
            QBrush(Qt::blue)
        );
    }
    
    // Show the note
    m_noteRect->setVisible(true);
    
    // Determine color based on voice index
    QColor noteColor = QColor::fromHsv((voiceIndex * 60) % 360, 200, 230);
    m_noteRect->setBrush(QBrush(noteColor));
    
    // Update bounds based on duration if available
    double duration = note.getDuration();
    if (duration > 0) {
        QRectF rect = m_noteRect->rect();
        rect.setWidth(duration * GRID_UNIT);
        m_noteRect->setRect(rect);
    }
}

void GridCell::clear()
{
    m_note.reset();
    m_voiceIndex = -1;
    if (m_noteRect) {
        m_noteRect->setVisible(false);
    }
}

void GridCell::update(const QRectF& viewport, float zoomLevel)
{
    if (!m_noteRect) return;
    
    if (m_note) {
        // Note is present, ensure it's visible and properly styled
        m_noteRect->setVisible(true);
        
        // Update position and size
        double duration = m_note->getDuration();
        m_noteRect->setRect(
            m_position * GRID_UNIT,
            m_pitch * NOTE_HEIGHT,
            duration * GRID_UNIT,
            NOTE_HEIGHT
        );
        
        // Adjust pen width based on zoom level
        QPen pen = m_noteRect->pen();
        pen.setWidthF(1.0 / zoomLevel);
        m_noteRect->setPen(pen);
    } else {
        // No note, hide the rectangle
        m_noteRect->setVisible(false);
    }
}

} // namespace MusicTrainer::presentation