#pragma once

namespace MusicTrainer::presentation::GridConstants {
    constexpr double GRID_UNIT = 40.0;
    constexpr double NOTE_HEIGHT = 12.0;
    constexpr double GRID_ZOOM_BASE = 1.0;
    constexpr int OCTAVE_RANGE = 12;  // Standard MIDI octave size
    
    inline int getOctaveRange() { return OCTAVE_RANGE; }
}