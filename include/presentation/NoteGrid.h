#pragma once

#include <QGraphicsScene>
#include <map>
#include <memory>
#include <optional>
#include "domain/music/Note.h"
#include "domain/music/Voice.h"
#include "domain/music/Score.h"
#include "domain/music/Duration.h"
#include "presentation/GridConstants.h"

namespace MusicTrainer::presentation {

class GridCell;

/**
 * @brief Sparse grid model for storing and rendering musical notes
 * 
 * This class implements the grid model from the ViewportSpec, providing efficient storage
 * and rendering of musical content with sparse representation to optimize memory usage.
 */
class NoteGrid {
public:
    /**
     * @brief Structure defining the dimensions of the grid in musical units
     */
    struct GridDimensions {
        int minPitch;      // Lowest visible pitch (MIDI note number)
        int maxPitch;      // Highest visible pitch (MIDI note number)
        int startPosition; // Leftmost position (in beats)
        int endPosition;   // Rightmost position (in beats)
        MusicTrainer::music::Voice::TimeSignature timeSignature{4, MusicTrainer::music::Duration::createQuarter()}; // Time signature with default 4/4
    };

    // Grid constants from shared header
    static constexpr float GRID_UNIT = GridConstants::GRID_UNIT;
    static constexpr float NOTE_HEIGHT = GridConstants::NOTE_HEIGHT;

    explicit NoteGrid(QGraphicsScene* scene);
    ~NoteGrid();

    /**
     * @brief Get the current grid dimensions
     * @return Current grid dimensions
     */
    const GridDimensions& getDimensions() const;

    /**
     * @brief Set the current grid dimensions
     * @param dimensions New grid dimensions
     */
    void setDimensions(const GridDimensions& dimensions);

    /**
     * @brief Expand the grid vertically
     * @param minPitchDelta Change to min pitch (negative to expand downward)
     * @param maxPitchDelta Change to max pitch (positive to expand upward)
     */
    void expandVertical(int minPitchDelta, int maxPitchDelta);

    /**
     * @brief Expand the grid horizontally
     * @param amount Amount to expand by (in musical time units)
     */
    void expandHorizontal(int amount);

    /**
     * @brief Update the grid from a musical score
     * @param score Score to render
     */
    void updateFromScore(std::shared_ptr<MusicTrainer::music::Score> score);

    /**
     * @brief Add a note to the grid
     * @param note Note to add
     * @param voiceIndex Index of the voice this note belongs to
     */
    void addNote(const MusicTrainer::music::Note& note, int voiceIndex);

    /**
     * @brief Add a note to the grid
     * @param note Note to add
     * @param voiceIndex Index of the voice this note belongs to
     * @param position Position in musical time (in beats)
     */
    void addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position = 0);
    
    /**
     * @brief Update the visual grid lines
     * @param visibleRect Currently visible rectangle
     * @param zoomLevel Current zoom level
     */
    void updateGridLines(const QRectF& visibleRect, float zoomLevel);

    /**
     * @brief Update the entire grid region
     * @param bounds New grid bounds
     */
    void updateGrid(const QRectF& bounds);

    /**
     * @brief Clear all notes and grid lines
     */
    void clear();

    /**
     * @brief Compact unused regions to optimize memory usage
     * @param visibleRect Currently visible rectangle
     * @param bufferZone Additional area to keep loaded
     */
    void compactUnusedRegions(const QRectF& visibleRect, const QRectF& bufferZone);

    /**
     * @brief Check if a cell contains a note
     * @param position Position in musical time
     * @param pitch MIDI note number
     * @return true if a note exists at the specified location
     */
    bool hasNoteAt(int position, int pitch) const;

    /**
     * @brief Get the current note count in the grid
     * @return Number of notes currently in the grid
     */
    int getNoteCount() const;

    /**
     * @brief Show a preview indicator at the specified musical position
     * @param position Position in musical time
     * @param pitch MIDI note number
     */
    void showNotePreview(int position, int pitch);

    /**
     * @brief Hide the note preview indicator
     */
    void hideNotePreview();

private:
    /**
     * @brief Create a new grid cell or get an existing one
     * @param position Position in musical time
     * @param pitch MIDI note number
     * @return Pointer to the grid cell
     */
    GridCell* getOrCreateCell(int position, int pitch);

    /**
     * @brief Clear all grid line elements from the scene
     */
    void clearGridElements();
    
    /**
     * @brief Update the visual appearance of grid lines
     * @param majorLines Whether to update major grid lines
     */
    void updateGridLineItems(bool majorLines = true);

    /**
     * @brief Validate and constrain grid dimensions to maintain one octave
     * @param dimensions Dimensions to validate
     * @return Constrained dimensions that maintain one octave
     */
    GridDimensions validateDimensions(const GridDimensions& dimensions) const;

    /**
     * @brief Get the current octave range (minPitch to maxPitch)
     * @return Octave range in semitones (always 12)
     */
    static constexpr int getOctaveRange() { return 12; }

    // Graphics scene that this grid belongs to
    QGraphicsScene* m_scene;

    // Grid dimensions
    GridDimensions m_dimensions;

    // Sparse storage for grid cells (position -> pitch -> cell)
    std::map<int, std::map<int, std::unique_ptr<GridCell>>> m_gridCells;

    // Grid line visual items
    std::vector<QGraphicsItem*> m_horizontalLines;
    std::vector<QGraphicsItem*> m_verticalLines;
    std::vector<QGraphicsItem*> m_majorHorizontalLines;
    std::vector<QGraphicsItem*> m_majorVerticalLines;

    // Statistics for optimization
    int m_noteCount{0};

    // Preview indicator
    QGraphicsItem* m_previewIndicator{nullptr};
    
    /**
     * @brief Update the preview indicator position
     * @param position Position in musical time
     * @param pitch MIDI note number
     */
    void updatePreviewPosition(int position, int pitch);
};

} // namespace MusicTrainer::presentation