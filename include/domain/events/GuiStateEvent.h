#pragma once

#include "domain/events/Event.h"
#include <string>
#include <variant>

namespace MusicTrainer::music::events {

class GuiStateEvent : public Event {
public:
    enum class StateType {
        VIEWPORT_CHANGE,
        SCORE_DISPLAY_CHANGE,
        SELECTION_CHANGE,
        PLAYBACK_STATE_CHANGE,
        MIDI_DEVICE_CHANGE
    };

    struct ViewportState {
        float x;
        float y;
        float zoomLevel;
        bool preserveOctaveExpansion;
    };

    struct ScoreDisplayState {
        bool showMeasureNumbers;
        bool showKeySignature;
        bool showVoiceLabels;
        int fontSize;
    };

    struct SelectionState {
        int voiceIndex;
        int measureIndex;
        int noteIndex;
    };

    struct PlaybackState {
        bool isPlaying;
        bool isRecording;
        int tempo;
        bool metronomeEnabled;
    };

    struct MidiDeviceState {
        int inputDeviceIndex;
        int outputDeviceIndex;
        bool midiThrough;
        int latency;
    };

    using StateVariant = std::variant<
        ViewportState,
        ScoreDisplayState,
        SelectionState,
        PlaybackState,
        MidiDeviceState
    >;

    static std::unique_ptr<GuiStateEvent> create(StateType type, 
                                               const StateVariant& state,
                                               const std::string& source);

    StateType getStateType() const { return m_stateType; }
    const StateVariant& getState() const { return m_state; }
    const std::string& getSource() const { return m_source; }

private:
    GuiStateEvent(StateType type, const StateVariant& state, const std::string& source);

    StateType m_stateType;
    StateVariant m_state;
    std::string m_source;
};