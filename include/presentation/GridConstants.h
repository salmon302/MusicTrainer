#pragma once

namespace MusicTrainer::presentation {

// Constants for grid sizing (shared between ScoreView and NoteGrid)
struct GridConstants {
    static constexpr float GRID_UNIT = 25.0f;    // Pixels per beat
    static constexpr float NOTE_HEIGHT = 20.0f;   // Pixels per semitone
    static constexpr float GRID_ZOOM_BASE = 1.0f; // Base zoom level
};

} // namespace MusicTrainer::presentation