#pragma once

#include <variant>
#include <memory>
#include "domain/music/Note.h"

namespace MusicTrainer::presentation::grid {

enum class GridDirection {
    Up,
    Down,
    Right,
    Left
};

struct GridCoordinate {
    int position;  // X coordinate (time)
    int pitch;     // Y coordinate (MIDI note number)
};

struct GridRegion {
    int startPosition;
    int endPosition;
    int minPitch;
    int maxPitch;
};

using GridCellValue = std::variant<std::shared_ptr<MusicTrainer::music::Note>, int>;

struct GridCell {
    GridCoordinate coord;
    GridCellValue value;
};

} // namespace MusicTrainer::presentation::grid