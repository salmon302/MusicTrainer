#include "presentation/grid/DefaultGridModel.h"
#include <algorithm>

namespace MusicTrainer::presentation::grid {

DefaultGridModel::DefaultGridModel(int chunkSize)
    : m_storage(chunkSize)
    , m_dimensions{48, 72, 0, 16} // Default to 2 octaves vertically, 16 positions horizontally
{
}

void DefaultGridModel::setDimensions(const GridDimensions& dimensions) {
    m_dimensions = dimensions;
}

const GridDimensions& DefaultGridModel::getDimensions() const {
    return m_dimensions;
}

void DefaultGridModel::clear() {
    m_storage.clear();
}

void DefaultGridModel::setCellValue(const GridCoordinate& coord, const GridCellValue& value) {
    if (!isValidCoordinate(coord)) return;
    m_storage.insert(coord, value);
}

GridCellValue DefaultGridModel::getCellValue(const GridCoordinate& coord) const {
    if (!isValidCoordinate(coord) || !m_storage.contains(coord)) {
        return 0; // Return default value for empty cells
    }
    return m_storage.getValue(coord);
}

bool DefaultGridModel::hasCell(const GridCoordinate& coord) const {
    return isValidCoordinate(coord) && m_storage.contains(coord);
}

void DefaultGridModel::removeCell(const GridCoordinate& coord) {
    if (isValidCoordinate(coord)) {
        m_storage.remove(coord);
    }
}

std::vector<GridCell> DefaultGridModel::getCellsInRegion(const GridRegion& region) const {
    // Clamp the region to valid dimensions
    GridRegion clampedRegion {
        std::max(region.startPosition, m_dimensions.startPosition),
        std::min(region.endPosition, m_dimensions.endPosition),
        std::max(region.minPitch, m_dimensions.minPitch),
        std::min(region.maxPitch, m_dimensions.maxPitch)
    };
    
    return m_storage.getInRegion(clampedRegion);
}

void DefaultGridModel::setCellsInBatch(const std::vector<GridCell>& cells) {
    std::vector<GridCell> validCells;
    validCells.reserve(cells.size());
    
    // Filter out invalid coordinates
    for (const auto& cell : cells) {
        if (isValidCoordinate(cell.coord)) {
            validCells.push_back(cell);
        }
    }
    
    m_storage.insertBatch(validCells);
}

void DefaultGridModel::compactStorage(const GridRegion& activeRegion) {
    m_storage.compact(activeRegion);
}

size_t DefaultGridModel::getMemoryUsage() const {
    return m_storage.getMemoryUsage();
}

bool DefaultGridModel::isValidCoordinate(const GridCoordinate& coord) const {
    return coord.position >= m_dimensions.startPosition &&
           coord.position < m_dimensions.endPosition &&
           coord.pitch >= m_dimensions.minPitch &&
           coord.pitch < m_dimensions.maxPitch;
}

GridDimensions DefaultGridModel::validateDimensions(const GridDimensions& dimensions) const {
    // Ensure dimensions are valid (e.g., min < max, reasonable ranges)
    GridDimensions validDimensions = dimensions;
    
    // Ensure min pitch is less than max pitch
    if (validDimensions.minPitch >= validDimensions.maxPitch) {
        validDimensions.maxPitch = validDimensions.minPitch + 24; // At least 2 octaves
    }
    
    // Ensure start position is less than end position
    if (validDimensions.startPosition >= validDimensions.endPosition) {
        validDimensions.endPosition = validDimensions.startPosition + 16; // At least 16 positions
    }
    
    // Constrain to reasonable ranges
    validDimensions.minPitch = std::max(0, validDimensions.minPitch);
    validDimensions.maxPitch = std::min(127, validDimensions.maxPitch); // MIDI note range
    
    return validDimensions;
}

void DefaultGridModel::expandVertical(int minPitchDelta, int maxPitchDelta) {
    m_dimensions.minPitch -= minPitchDelta;
    m_dimensions.maxPitch += maxPitchDelta;
    
    // Validate the new dimensions
    m_dimensions = validateDimensions(m_dimensions);
}

void DefaultGridModel::expandHorizontal(int amount) {
    // Expand in both directions
    m_dimensions.startPosition -= amount / 2;
    m_dimensions.endPosition += amount - (amount / 2);
}

void DefaultGridModel::addNote(const MusicTrainer::music::Note& note, int voiceIndex, int position) {
    GridCoordinate coord{position, note.getPitch().getMidiNote()};
    setCellValue(coord, std::make_shared<MusicTrainer::music::Note>(note));
}

bool DefaultGridModel::hasNoteAt(int position, int pitch) const {
    GridCoordinate coord{position, pitch};
    return hasCell(coord) && std::holds_alternative<std::shared_ptr<MusicTrainer::music::Note>>(getCellValue(coord));
}

std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>>
DefaultGridModel::getNotesInRange(int minPos, int maxPos, int minPitch, int maxPitch) const {
    GridRegion region{minPos, maxPos, minPitch, maxPitch};
    auto cells = getCellsInRegion(region);
    
    std::vector<std::tuple<const MusicTrainer::music::Note*, int, int>> result;
    result.reserve(cells.size());
    
    for (const auto& cell : cells) {
        if (auto notePtr = std::get_if<std::shared_ptr<MusicTrainer::music::Note>>(&cell.value)) {
            if (*notePtr) {
                result.emplace_back(notePtr->get(), cell.coord.position, cell.coord.pitch);
            }
        }
    }
    
    return result;
}

int DefaultGridModel::removeNotesInRange(int startPos, int endPos) {
    GridRegion region{startPos, endPos, m_dimensions.minPitch, m_dimensions.maxPitch};
    auto cells = getCellsInRegion(region);
    
    int removedCount = 0;
    for (const auto& cell : cells) {
        if (std::holds_alternative<std::shared_ptr<MusicTrainer::music::Note>>(cell.value)) {
            removeCell(cell.coord);
            removedCount++;
        }
    }
    
    return removedCount;
}

int DefaultGridModel::getNoteCount() const {
    GridRegion fullRegion{
        m_dimensions.startPosition,
        m_dimensions.endPosition,
        m_dimensions.minPitch,
        m_dimensions.maxPitch
    };
    
    auto cells = getCellsInRegion(fullRegion);
    
    int count = 0;
    for (const auto& cell : cells) {
        if (std::holds_alternative<std::shared_ptr<MusicTrainer::music::Note>>(cell.value)) {
            count++;
        }
    }
    
    return count;
}

void DefaultGridModel::compactUnusedRegions(const Rectangle& visibleRect, const Rectangle& bufferZone) {
    // Convert screen coordinates to grid coordinates
    GridRegion activeRegion{
        static_cast<int>(visibleRect.x - bufferZone.width),
        static_cast<int>(visibleRect.x + visibleRect.width + bufferZone.width),
        static_cast<int>(visibleRect.y - bufferZone.height),
        static_cast<int>(visibleRect.y + visibleRect.height + bufferZone.height)
    };
    
    // Use the existing compactStorage method
    compactStorage(activeRegion);
}

} // namespace MusicTrainer::presentation::grid