#pragma once

#include "IGridModel.h"
#include "presentation/NoteGrid.h"
#include <memory>

namespace MusicTrainer::presentation::grid {

/**
 * @brief Adapter class that wraps NoteGrid and implements IGridModel
 * 
 * This adapter allows the existing NoteGrid implementation to be used
 * with the new grid interfaces, facilitating incremental migration.
 */
class NoteGridAdapter : public IGridModel {
public:
    explicit NoteGridAdapter(MusicTrainer::presentation::NoteGrid* grid);
    ~NoteGridAdapter() override = default;
    
    // IGridModel implementation
    void setDimensions(const GridDimensions& dimensions) override;
    const GridDimensions& getDimensions() const override;
    GridDimensions validateDimensions(const GridDimensions& dimensions) const override;
    void expandVertical(int minPitchDelta, int maxPitchDelta) override;
    void expandHorizontal(int amount) override;
    void addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position) override;
    bool hasNoteAt(int position, int pitch) const override;
    std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>> 
    getNotesInRange(int minPos, int maxPos, int minPitch, int maxPitch) const override;
    int removeNotesInRange(int startPos, int endPos) override;
    void clear() override;
    int getNoteCount() const override;
    void compactUnusedRegions(const Rectangle& visibleRect, const Rectangle& bufferZone) override;
    
    // Cell management
    void setCellValue(const GridCoordinate& coord, const GridCellValue& value) override;
    GridCellValue getCellValue(const GridCoordinate& coord) const override;
    bool hasCell(const GridCoordinate& coord) const override;
    void removeCell(const GridCoordinate& coord) override;
    
    // Batch operations
    std::vector<GridCell> getCellsInRegion(const GridRegion& region) const override;
    void setCellsInBatch(const std::vector<GridCell>& cells) override;
    
    // Access to the wrapped NoteGrid
    MusicTrainer::presentation::NoteGrid* getNoteGrid() const { return m_grid; }
    
private:
    MusicTrainer::presentation::NoteGrid* m_grid;
    mutable GridDimensions m_cachedDimensions;
    bool m_modified;  // Track modification state
    
    // Helper methods for coordinate conversion
    GridCoordinate noteGridToGridCoord(int position, int pitch) const;
    std::pair<int, int> gridCoordToNoteGrid(const GridCoordinate& coord) const;
    GridRegion rectangleToGridRegion(const Rectangle& rect) const;
    QRectF gridRegionToQRectF(const GridRegion& region) const;
};

} // namespace MusicTrainer::presentation::grid