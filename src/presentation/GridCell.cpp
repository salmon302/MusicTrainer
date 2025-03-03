#include "presentation/GridCell.h"
#include <QDebug>

// Import music namespace for consistency
using music::Note;

namespace MusicTrainer::presentation {

GridCell::GridCell(int position, int pitch, QGraphicsScene* scene)
    : m_position(position)
    , m_pitch(pitch)
    , m_scene(scene)
    , m_noteRect(nullptr)
    , m_voiceIndex(0)
{
    // Create default note rectangle
    if (m_scene) {
        // Create as QGraphicsRectItem for proper styling support
        m_noteRect = m_scene->addRect(
            m_position * GRID_UNIT,
            m_pitch * NOTE_HEIGHT,
            GRID_UNIT,
            NOTE_HEIGHT
        );
        
        // Cast to QGraphicsRectItem to access pen and brush methods
        auto* rectItem = static_cast<QGraphicsRectItem*>(m_noteRect);
        rectItem->setPen(QPen(Qt::black));
        rectItem->setBrush(QBrush(QColor(100, 100, 240)));
        
        // Hide by default until a note is added
        m_noteRect->setVisible(false);
    }
}

GridCell::~GridCell()
{
    if (m_scene && m_noteRect) {
        m_scene->removeItem(m_noteRect);
        delete m_noteRect;
    }
}

bool GridCell::isVisible(const QRectF& viewport) const
{
    if (!m_noteRect) return false;
    
    QRectF bounds = m_noteRect->boundingRect();
    bounds.moveTopLeft(m_noteRect->pos());
    return viewport.intersects(bounds);
}

void GridCell::setNote(const Note& note, int voiceIndex)
{
    m_note = note;
    m_voiceIndex = voiceIndex;
    
    if (m_noteRect) {
        // Get proper duration in beats for width
        float width = note.getDuration() * GRID_UNIT;
        
        // Update rectangle size
        auto* rectItem = static_cast<QGraphicsRectItem*>(m_noteRect);
        rectItem->setRect(
            m_position * GRID_UNIT,
            m_pitch * NOTE_HEIGHT,
            width,
            NOTE_HEIGHT
        );
        
        // Make visible
        m_noteRect->setVisible(true);
        
        // Update appearance
        QColor noteColor;
        switch (m_voiceIndex % 4) {
            case 0: noteColor = QColor(100, 100, 240); break; // Blue
            case 1: noteColor = QColor(100, 240, 100); break; // Green
            case 2: noteColor = QColor(240, 100, 100); break; // Red
            case 3: noteColor = QColor(240, 240, 100); break; // Yellow
        }
        
        rectItem->setBrush(QBrush(noteColor));
    }
}

void GridCell::clear()
{
    if (m_noteRect) {
        m_noteRect->setVisible(false);
    }
    m_note.reset();
}

void GridCell::update(const QRectF& viewport, float zoomLevel)
{
    if (!m_noteRect || !m_note) return;
    
    auto* rectItem = static_cast<QGraphicsRectItem*>(m_noteRect);
    QColor baseColor = rectItem->brush().color();
    
    // Update visibility
    bool isVisible = this->isVisible(viewport);
    m_noteRect->setVisible(isVisible);
    
    if (isVisible) {
        // Adjust pen width based on zoom
        if (zoomLevel != 1.0f) {
            float penWidth = 1.0f / zoomLevel;
            QPen pen = rectItem->pen();
            pen.setWidthF(penWidth);
            rectItem->setPen(pen);
        }
        
        // Update position and size
        float width = m_note->getDuration() * GRID_UNIT;
        rectItem->setRect(
            m_position * GRID_UNIT,
            m_pitch * NOTE_HEIGHT,
            width,
            NOTE_HEIGHT
        );
    }
}

} // namespace MusicTrainer::presentation