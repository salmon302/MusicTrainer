#pragma once

#include <vector>
#include "domain/music/Note.h"
#include "presentation/grid/GridTypes.h"

namespace MusicTrainer::presentation::grid {

/**
 * @brief Simple rectangle structure for grid regions
 */
struct Rectangle {
    float x;
    float y;
    float width;
    float height;
    
    bool contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
    
    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
};

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

/**
 * @brief Interface for grid data model
 * 
 * This interface defines the contract for grid data models, providing
 * methods for managing grid dimensions, adding notes, and querying
 * the grid content.
 */
class IGridModel {
public:
    virtual ~IGridModel() = default;
    
    /**
     * @brief Set grid dimensions
     * @param dimensions New dimensions
     */
    virtual void setDimensions(const GridDimensions& dimensions) = 0;
    
    /**
     * @brief Get current grid dimensions
     * @return Grid dimensions
     */
    virtual const GridDimensions& getDimensions() const = 0;
    
    /**
     * @brief Validate and constrain grid dimensions
     * @param dimensions Dimensions to validate
     * @return Constrained dimensions
     */
    virtual GridDimensions validateDimensions(const GridDimensions& dimensions) const = 0;
    
    /**
     * @brief Expand grid vertically
     * @param minPitchDelta Change to apply to minPitch
     * @param maxPitchDelta Change to apply to maxPitch
     */
    virtual void expandVertical(int minPitchDelta, int maxPitchDelta) = 0;
    
    /**
     * @brief Expand grid horizontally
     * @param amount Amount to expand by
     */
    virtual void expandHorizontal(int amount) = 0;
    
    /**
     * @brief Add a note to the grid
     * @param note Note to add
     * @param voiceIndex Voice index
     * @param position Position to add at
     */
    virtual void addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position) = 0;
    
    /**
     * @brief Check if there is a note at the given position and pitch
     * @param position Position to check
     * @param pitch Pitch to check
     * @return True if note exists
     */
    virtual bool hasNoteAt(int position, int pitch) const = 0;
    
    /**
     * @brief Get notes in a specific range
     * @param minPos Minimum position (inclusive)
     * @param maxPos Maximum position (exclusive)
     * @param minPitch Minimum pitch (inclusive)
     * @param maxPitch Maximum pitch (exclusive)
     * @return Vector of notes in the range
     */
    virtual std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>> 
    getNotesInRange(int minPos, int maxPos, int minPitch, int maxPitch) const = 0;
    
    /**
     * @brief Remove notes within a specific position range
     * @param startPos Starting position (inclusive)
     * @param endPos Ending position (exclusive)
     * @return Number of notes removed
     */
    virtual int removeNotesInRange(int startPos, int endPos) = 0;
    
    /**
     * @brief Clear all grid content
     */
    virtual void clear() = 0;
    
    /**
     * @brief Get total note count
     * @return Number of notes in the grid
     */
    virtual int getNoteCount() const = 0;
    
    /**
     * @brief Free memory for regions outside the buffer zone
     * @param visibleRect Currently visible rectangle
     * @param bufferZone Buffer zone around visible area
     */
    virtual void compactUnusedRegions(const Rectangle& visibleRect, const Rectangle& bufferZone) = 0;

    // Cell management
    virtual void setCellValue(const GridCoordinate& coord, const GridCellValue& value) = 0;
    virtual GridCellValue getCellValue(const GridCoordinate& coord) const = 0;
    virtual bool hasCell(const GridCoordinate& coord) const = 0;
    virtual void removeCell(const GridCoordinate& coord) = 0;

    // Batch operations
    virtual std::vector<GridCell> getCellsInRegion(const GridRegion& region) const = 0;
    virtual void setCellsInBatch(const std::vector<GridCell>& cells) = 0;
};

} // namespace MusicTrainer::presentation::grid