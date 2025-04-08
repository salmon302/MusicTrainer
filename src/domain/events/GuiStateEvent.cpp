#include "domain/events/GuiStateEvent.h"
#include <sstream>
#include <variant>

namespace MusicTrainer::music::events {

std::unique_ptr<GuiStateEvent> GuiStateEvent::create(
    StateType type,
    const StateVariant& state,
    const std::string& source
) {
    // Use private constructor through unique_ptr construction
    return std::unique_ptr<GuiStateEvent>(new GuiStateEvent(type, state, source));
}

GuiStateEvent::GuiStateEvent(StateType type, const StateVariant& state, const std::string& source)
    : m_stateType(type)
    , m_state(state)
    , m_source(source)
{}

std::string GuiStateEvent::getDescription() const {
    std::ostringstream oss;
    oss << "GUI State Change - ";
    
    switch (m_stateType) {
        case StateType::VIEWPORT_CHANGE:
            oss << "Viewport";
            break;
        case StateType::SCORE_DISPLAY_CHANGE:
            oss << "Score Display";
            break;
        case StateType::SELECTION_CHANGE:
            oss << "Selection";
            break;
        case StateType::PLAYBACK_STATE_CHANGE:
            oss << "Playback State";
            break;
        case StateType::MIDI_DEVICE_CHANGE:
            oss << "MIDI Device";
            break;
    }
    
    if (!m_source.empty()) {
        oss << " from " << m_source;
    }
    
    // Add specific state details based on variant type
    std::visit([&oss](const auto& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, ViewportState>) {
            oss << " [pos: " << s.x << "," << s.y 
                << " zoom: " << s.zoomLevel << "]";
        }
        else if constexpr (std::is_same_v<T, PlaybackState>) {
            oss << " [" << (s.isPlaying ? "playing" : "stopped")
                << " tempo: " << s.tempo << "]";
        }
        else if constexpr (std::is_same_v<T, SelectionState>) {
            oss << " [voice: " << s.voiceIndex 
                << " measure: " << s.measureIndex
                << " note: " << s.noteIndex << "]";
        }
    }, m_state);
    
    return oss.str();
}

std::unique_ptr<Event> GuiStateEvent::clone() const {
    return std::unique_ptr<Event>(new GuiStateEvent(m_stateType, m_state, m_source));
}

} // namespace MusicTrainer::music::events