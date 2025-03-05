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
     * @brief Grid dimensions representing the visible area
     */
    struct GridDimensions {
        int minPitch;       // Lowest visible pitch (MIDI note number)
        int maxPitch;       // Highest visible pitch (MIDI note number)
        int startPosition;  // Leftmost visible position (time)
        int endPosition;    // Rightmost visible position (time)
        float timeSignature{4.0f}; // Time signature (beats per measure)
    };

    // Grid constants from shared header
    static constexpr float GRID_UNIT = GridConstants::GRID_UNIT;
    static constexpr float NOTE_HEIGHT = GridConstants::NOTE_HEIGHT;

    /**
     * @brief Constructor taking a scene to render in
     * @param scene The scene to add grid elements to
     */
    explicit NoteGrid(QGraphicsScene* scene);
    
    /**
     * @brief Destructor
     */
    ~NoteGrid();
    
    /**
     * @brief Get current grid dimensions
     * @return Grid dimensions
     */
    const GridDimensions& getDimensions() const;
    
    /**
     * @brief Validate and constrain grid dimensions
     * @param dimensions Dimensions to validate
     * @return Constrained dimensions
     */
    GridDimensions validateDimensions(const GridDimensions& dimensions) const;

    /**
     * @brief Get the current octave range (minPitch to maxPitch)
     * @return Octave range in semitones (default 12, can be expanded)
     */
    static constexpr int getOctaveRange() { return 12; }
    
    /**
     * @brief Get the minimum octave range
     * @return Minimum octave range that must be shown (12 semitones)
     */
    static constexpr int getMinimumOctaveRange() { return 12; }
    
    /**
     * @brief Set grid dimensions
     * @param dimensions New dimensions
     */
    void setDimensions(const GridDimensions& dimensions);
    
    /**
     * @brief Expand grid vertically
     * @param minPitchDelta Change to apply to minPitch
     * @param maxPitchDelta Change to apply to maxPitch
     */
    void expandVertical(int minPitchDelta, int maxPitchDelta);
    
    /**
     * @brief Expand grid horizontally
     * @param amount Amount to expand by
     */
    void expandHorizontal(int amount);
    
    /**
     * @brief Update grid based on a score
     * @param score Score to use for updating
     */
    void updateFromScore(std::shared_ptr<MusicTrainer::music::Score> score);
    
    /**
     * @brief Add a note to the grid
     * @param note Note to add
     * @param voiceIndex Voice index
     */
    void addNote(const MusicTrainer::music::Note& note, int voiceIndex);
    
    /**
     * @brief Add a note to the grid at a specific position
     * @param note Note to add
     * @param voiceIndex Voice index
     * @param position Position to add at
     */
    void addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position);
    
    /**
     * @brief Update grid lines for the visible rectangle
     * @param visibleRect Visible rectangle
     * @param zoomLevel Current zoom level
     */
    void updateGridLines(const QRectF& visibleRect, float zoomLevel);
    
    /**
     * @brief Update grid cells for the visible bounds
     * @param bounds Visible bounds
     */
    void updateGrid(const QRectF& bounds);
    
    /**
     * @brief Clear all grid content
     */
    void clear();
    
    /**
     * @brief Free memory for regions outside the buffer zone
     * @param visibleRect Currently visible rectangle
     * @param bufferZone Buffer zone around visible area
     */
    void compactUnusedRegions(const QRectF& visibleRect, const QRectF& bufferZone);
    
    /**
     * @brief Check if there is a note at the given position and pitch
     * @param position Position to check
     * @param pitch Pitch to check
     * @return True if note exists
     */
    bool hasNoteAt(int position, int pitch) const;
    
    /**
     * @brief Get total note count
     * @return Number of notes in the grid
     */
    int getNoteCount() const;
    
    /**
     * @brief Show a note preview at the specified position
     * @param position Position for the preview
     * @param pitch Pitch for the preview
     */
    void showNotePreview(int position, int pitch);
    
    /**
     * @brief Hide the note preview
     */
    void hideNotePreview();
    
    /**
     * @brief Get the scene this grid renders to
     * @return The scene pointer
     */
    QGraphicsScene* getScene() const { return m_scene; }

private:
    /**
     * @brief Get or create a cell at the specified position and pitch
     * @param position Position to get/create
     * @param pitch Pitch to get/create
     * @return Pointer to the cell
     */
    GridCell* getOrCreateCell(int position, int pitch);
    
    /**
     * @brief Update grid line items
     * @param majorLines Whether to update major lines
     */
    void updateGridLineItems(bool majorLines);
    
    /**
     * @brief Clear grid elements but keep note cells
     */
    void clearGridElements();
    
    /**
     * @brief Update the position of the note preview
     * @param position Position for the preview
     * @param pitch Pitch for the preview
     */
    void updatePreviewPosition(int position, int pitch);

    // Graphics scene that this grid belongs to
    QGraphicsScene* m_scene;

    // Grid dimensions
    GridDimensions m_dimensions;

    // Sparse storage for grid cells (position -> pitch -> cell)
    std::map<int, std::map<int, std::unique_ptr<GridCell>>> m_gridCells;
    
    // Cache of horizontal grid lines
    std::vector<QGraphicsItem*> m_horizontalLines;
    
    // Cache of vertical grid lines
    std::vector<QGraphicsItem*> m_verticalLines;
    
    // Cache of major horizontal grid lines (octave boundaries)
    std::vector<QGraphicsItem*> m_majorHorizontalLines;
    
    // Cache of major vertical grid lines (measure boundaries)
    std::vector<QGraphicsItem*> m_majorVerticalLines;
    
    // Preview indicator for note placement
    QGraphicsItem* m_previewIndicator{nullptr};
    
    // Note count for statistics
    int m_noteCount{0};
};

} // namespace MusicTrainer::presentation