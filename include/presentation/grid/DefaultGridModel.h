#pragma once

#include "IGridModel.h"
#include "SpatialIndex.h"

namespace MusicTrainer::presentation::grid {

class DefaultGridModel : public IGridModel {
public:
    explicit DefaultGridModel(int chunkSize = 16);
    ~DefaultGridModel() override = default;

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
    void setCellValue(const GridCoordinate& coord, const GridCellValue& value) override;
    GridCellValue getCellValue(const GridCoordinate& coord) const override;
    bool hasCell(const GridCoordinate& coord) const override;
    void removeCell(const GridCoordinate& coord) override;
    std::vector<GridCell> getCellsInRegion(const GridRegion& region) const override;
    void setCellsInBatch(const std::vector<GridCell>& cells) override;

    // Additional functionality
    void compactStorage(const GridRegion& activeRegion);
    size_t getMemoryUsage() const;
    SpatialIndex& getSpatialIndex() { return m_storage; }
    const SpatialIndex& getSpatialIndex() const { return m_storage; }

private:
    SpatialIndex m_storage;
    GridDimensions m_dimensions;

    bool isValidCoordinate(const GridCoordinate& coord) const;
};

} // namespace MusicTrainer::presentation::grid