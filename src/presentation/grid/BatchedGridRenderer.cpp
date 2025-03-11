#include "presentation/grid/BatchedGridRenderer.h"
#include <algorithm>

namespace MusicTrainer::presentation::grid {

BatchedGridRenderer::BatchedGridRenderer()
    : m_viewportWidth(0)
    , m_viewportHeight(0)
    , m_scale(1.0f)
{
}

void BatchedGridRenderer::beginBatch(const Rectangle& visibleRect) {
    m_visibleRect = visibleRect;
    while (!m_renderQueue.empty()) {
        m_renderQueue.pop();
    }
}

void BatchedGridRenderer::renderCell(const GridCell& cell, const IGridModel& model) {
    if (isCellVisible(cell)) {
        m_renderQueue.push({cell, std::cref(model), false});
    }
}

// Define a proper DummyModel class outside the method
class DummyModel : public IGridModel {
public:
    void setDimensions(const GridDimensions& dimensions) override {}
    const GridDimensions& getDimensions() const override { static GridDimensions d; return d; }
    GridDimensions validateDimensions(const GridDimensions& dimensions) const override { return dimensions; }
    void expandVertical(int minPitchDelta, int maxPitchDelta) override {}
    void expandHorizontal(int amount) override {}
    void addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position) override {}
    bool hasNoteAt(int position, int pitch) const override { return false; }
    std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>>
    getNotesInRange(int minPos, int maxPos, int minPitch, int maxPitch) const override {
        return {};
    }
    int removeNotesInRange(int startPos, int endPos) override { return 0; }
    void clear() override {}
    int getNoteCount() const override { return 0; }
    void compactUnusedRegions(const Rectangle& visibleRect, const Rectangle& bufferZone) override {}
    void setCellValue(const GridCoordinate& coord, const GridCellValue& value) override {}
    GridCellValue getCellValue(const GridCoordinate& coord) const override { return 0; }
    bool hasCell(const GridCoordinate& coord) const override { return false; }
    void removeCell(const GridCoordinate& coord) override {}
    std::vector<GridCell> getCellsInRegion(const GridRegion& region) const override { return {}; }
    void setCellsInBatch(const std::vector<GridCell>& cells) override {}
};

void BatchedGridRenderer::renderPreview(const GridCell& cell) {
    if (isCellVisible(cell)) {
        // Create a dummy model reference - the preview doesn't use it
        static DummyModel dummyModel;
        m_renderQueue.push({cell, std::cref(dummyModel), true});
    }
}

void BatchedGridRenderer::endBatch() {
    processRenderQueue();
}

void BatchedGridRenderer::clear() {
    while (!m_renderQueue.empty()) {
        m_renderQueue.pop();
    }
}

void BatchedGridRenderer::setViewportSize(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
}

void BatchedGridRenderer::setScale(float scale) {
    m_scale = scale;
}

void BatchedGridRenderer::processRenderQueue() {
    while (!m_renderQueue.empty()) {
        const auto& cmd = m_renderQueue.front();
        
        // Process differently based on whether it's a preview or regular cell
        if (cmd.isPreview) {
            // Preview cells are typically implemented by derived classes
            // Base class just provides a queue mechanism
        } else {
            // Regular cell rendering
            // TODO: Implement actual rendering logic using Qt
        }
        
        m_renderQueue.pop();
    }
}

bool BatchedGridRenderer::isCellVisible(const GridCell& cell) const {
    // Convert cell coordinates to screen space and check visibility
    return m_visibleRect.contains(
        static_cast<float>(cell.coord.position),
        static_cast<float>(cell.coord.pitch)
    );
}

} // namespace MusicTrainer::presentation::grid