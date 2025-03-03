#pragma once

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include "domain/music/Note.h"
#include "presentation/GridConstants.h"
#include <optional>

// Import the music namespace for consistency
using music::Note;

namespace MusicTrainer::presentation {

/**
 * @brief Represents a cell in the musical grid that can contain a note
 */
class GridCell {
public:
    // Use shared grid constants
    static constexpr float GRID_UNIT = GridConstants::GRID_UNIT;
    static constexpr float NOTE_HEIGHT = GridConstants::NOTE_HEIGHT;

    /**
     * @brief Construct a new grid cell
     * @param position Musical time position (in beats)
     * @param pitch MIDI note number
     * @param scene Graphics scene to add visuals to
     */
    GridCell(int position, int pitch, QGraphicsScene* scene);
    ~GridCell();

    /**
     * @brief Get the cell's position in musical time
     */
    int getPosition() const { return m_position; }

    /**
     * @brief Get the cell's pitch
     */
    int getPitch() const { return m_pitch; }

    /**
     * @brief Check if the cell is visible in the viewport
     * @param viewport Current viewport rectangle
     */
    bool isVisible(const QRectF& viewport) const;

    /**
     * @brief Set the note in this cell
     * @param note Note to place in the cell
     * @param voiceIndex Voice index for coloring
     */
    void setNote(const Note& note, int voiceIndex);

    /**
     * @brief Clear any note from this cell
     */
    void clear();

    /**
     * @brief Update the cell's visual appearance
     * @param viewport Current viewport rectangle
     * @param zoomLevel Current zoom level
     */
    void update(const QRectF& viewport, float zoomLevel);

private:
    int m_position;
    int m_pitch;
    QGraphicsScene* m_scene;
    QGraphicsItem* m_noteRect;
    std::optional<Note> m_note;
    int m_voiceIndex;
};

} // namespace MusicTrainer::presentation