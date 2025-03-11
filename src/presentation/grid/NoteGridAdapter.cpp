#include "presentation/grid/NoteGridAdapter.h"
#include <QRectF>
#include <QDebug>

namespace MusicTrainer::presentation::grid {

NoteGridAdapter::NoteGridAdapter(MusicTrainer::presentation::NoteGrid* grid)
    : m_grid(grid)
    , m_modified(false)
{
    // Initialize cached dimensions from the NoteGrid
    auto noteGridDims = m_grid->getDimensions();
    m_cachedDimensions = {
        noteGridDims.minPitch,
        noteGridDims.maxPitch,
        noteGridDims.startPosition,
        noteGridDims.endPosition,
        noteGridDims.timeSignature
    };
}

void NoteGridAdapter::setDimensions(const GridDimensions& dimensions) {
    MusicTrainer::presentation::NoteGrid::GridDimensions noteGridDims{
        dimensions.minPitch,
        dimensions.maxPitch,
        dimensions.startPosition,
        dimensions.endPosition,
        dimensions.timeSignature
    };
    
    m_grid->setDimensions(noteGridDims);
    m_cachedDimensions = dimensions;
}

const GridDimensions& NoteGridAdapter::getDimensions() const {
    // Update cached dimensions from the NoteGrid
    auto noteGridDims = m_grid->getDimensions();
    m_cachedDimensions = {
        noteGridDims.minPitch,
        noteGridDims.maxPitch,
        noteGridDims.startPosition,
        noteGridDims.endPosition,
        noteGridDims.timeSignature
    };
    
    return m_cachedDimensions;
}

GridDimensions NoteGridAdapter::validateDimensions(const GridDimensions& dimensions) const {
    MusicTrainer::presentation::NoteGrid::GridDimensions noteGridDims{
        dimensions.minPitch,
        dimensions.maxPitch,
        dimensions.startPosition,
        dimensions.endPosition,
        dimensions.timeSignature
    };
    
    auto validatedNoteGridDims = m_grid->validateDimensions(noteGridDims);
    
    return {
        validatedNoteGridDims.minPitch,
        validatedNoteGridDims.maxPitch,
        validatedNoteGridDims.startPosition,
        validatedNoteGridDims.endPosition,
        validatedNoteGridDims.timeSignature
    };
}

void NoteGridAdapter::expandVertical(int minPitchDelta, int maxPitchDelta) {
    m_grid->expandVertical(minPitchDelta, maxPitchDelta);
    
    // Update cached dimensions
    auto noteGridDims = m_grid->getDimensions();
    m_cachedDimensions = {
        noteGridDims.minPitch,
        noteGridDims.maxPitch,
        noteGridDims.startPosition,
        noteGridDims.endPosition,
        noteGridDims.timeSignature
    };
}

void NoteGridAdapter::expandHorizontal(int amount) {
    m_grid->expandHorizontal(amount);
    
    // Update cached dimensions
    auto noteGridDims = m_grid->getDimensions();
    m_cachedDimensions = {
        noteGridDims.minPitch,
        noteGridDims.maxPitch,
        noteGridDims.startPosition,
        noteGridDims.endPosition,
        noteGridDims.timeSignature
    };
}

void NoteGridAdapter::addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position) {
    m_grid->addNote(note, voiceIndex, position);
}

bool NoteGridAdapter::hasNoteAt(int position, int pitch) const {
    return m_grid->hasNoteAt(position, pitch);
}

std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>> 
NoteGridAdapter::getNotesInRange(int startPos, int endPos, int minPitch, int maxPitch) const {
    QRectF regionRect(startPos, minPitch, endPos - startPos, maxPitch - minPitch);
    std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>> result;
    
    // Iterate through positions to find notes
    for (int pos = startPos; pos < endPos; ++pos) {
        for (int pitch = minPitch; pitch <= maxPitch; ++pitch) {
            if (m_grid->hasNoteAt(pos, pitch)) {
                // Note: Since we can't directly access the Note object from hasNoteAt,
                // we'll create a temporary Note. In a real implementation, you would
                // want to add a method to NoteGrid to get the actual Note object.
                auto note = MusicTrainer::music::Note(
                    MusicTrainer::music::Pitch::fromMidiNote(pitch),
                    1.0,  // Default duration
                    pos
                );
                result.emplace_back(&note, pos, pitch);
            }
        }
    }
    
    return result;
}

int NoteGridAdapter::removeNotesInRange(int startPos, int endPos) {
    return m_grid->removeNotesInRange(startPos, endPos);
}

void NoteGridAdapter::clear() {
    m_grid->clear();
}

int NoteGridAdapter::getNoteCount() const {
    return m_grid->getNoteCount();
}

void NoteGridAdapter::compactUnusedRegions(const Rectangle& visibleRect, const Rectangle& bufferZone) {
    QRectF qVisibleRect(visibleRect.x, visibleRect.y, visibleRect.width, visibleRect.height);
    QRectF qBufferZone(bufferZone.x, bufferZone.y, bufferZone.width, bufferZone.height);
    
    m_grid->compactUnusedRegions(qVisibleRect, qBufferZone);
}

void NoteGridAdapter::setCellValue(const GridCoordinate& coord, const GridCellValue& value) {
    if (auto notePtr = std::get_if<std::shared_ptr<MusicTrainer::music::Note>>(&value)) {
        if (*notePtr) {
            MusicTrainer::music::Note note = **notePtr;
            m_grid->addNote(note, 0, coord.position);  // Using voice index 0
            m_modified = true;
        }
    }
}

GridCellValue NoteGridAdapter::getCellValue(const GridCoordinate& coord) const {
    // Check if there's a note at this position/pitch
    if (m_grid->hasNoteAt(coord.position, coord.pitch)) {
        // Since we don't have direct access to the note object,
        // we'll create a value representation
        // Return 1 to indicate the presence of a note
        return 1;
    }
    
    // No note at this coordinate
    return 0;
}

bool NoteGridAdapter::hasCell(const GridCoordinate& coord) const {
    return m_grid->hasNoteAt(coord.position, coord.pitch);
}

void NoteGridAdapter::removeCell(const GridCoordinate& coord) {
    // This is a placeholder implementation since NoteGrid doesn't have a direct equivalent
    // In a real implementation, you would need to access the internal data structure of NoteGrid
    // and remove the specific cell
    
    // For now, we'll use removeNotesInRange as an approximation
    m_grid->removeNotesInRange(coord.position, coord.position + 1);
}

std::vector<GridCell> NoteGridAdapter::getCellsInRegion(const GridRegion& region) const {
    std::vector<GridCell> result;
    
    for (int pos = region.startPosition; pos < region.endPosition; ++pos) {
        for (int pitch = region.minPitch; pitch <= region.maxPitch; ++pitch) {
            if (m_grid->hasNoteAt(pos, pitch)) {
                GridCoordinate coord{pos, pitch};
                // Create a note with default duration since we can't access the actual note
                auto note = std::make_shared<MusicTrainer::music::Note>(
                    MusicTrainer::music::Pitch::fromMidiNote(pitch),
                    1.0,  // Default duration
                    pos
                );
                result.emplace_back(GridCell{coord, note});
            }
        }
    }
    
    return result;
}

void NoteGridAdapter::setCellsInBatch(const std::vector<GridCell>& cells) {
    // Update the grid with current bounds to ensure we update everything
    m_grid->updateGrid(m_grid->getScene()->sceneRect());
    
    for (const auto& cell : cells) {
        if (auto notePtr = std::get_if<std::shared_ptr<MusicTrainer::music::Note>>(&cell.value)) {
            if (*notePtr) {
                MusicTrainer::music::Note note = **notePtr;
                m_grid->addNote(note, 0, cell.coord.position);  // Using voice index 0
            }
        }
    }
    
    m_modified = true;
}

GridCoordinate NoteGridAdapter::noteGridToGridCoord(int position, int pitch) const {
    return {position, pitch};
}

std::pair<int, int> NoteGridAdapter::gridCoordToNoteGrid(const GridCoordinate& coord) const {
    return {coord.position, coord.pitch};
}

GridRegion NoteGridAdapter::rectangleToGridRegion(const Rectangle& rect) const {
    // Convert screen coordinates to grid coordinates
    auto topLeft = noteGridToGridCoord(static_cast<int>(rect.x), static_cast<int>(rect.y));
    auto bottomRight = noteGridToGridCoord(static_cast<int>(rect.x + rect.width), 
                                          static_cast<int>(rect.y + rect.height));
    
    return {
        topLeft.position,
        bottomRight.position,
        bottomRight.pitch,  // Note: In grid coordinates, y increases downward, so min/max are reversed
        topLeft.pitch
    };
}

QRectF NoteGridAdapter::gridRegionToQRectF(const GridRegion& region) const {
    return QRectF(
        region.startPosition,
        region.minPitch,
        region.endPosition - region.startPosition,
        region.maxPitch - region.minPitch
    );
}

} // namespace MusicTrainer::presentation::grid