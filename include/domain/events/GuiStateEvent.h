#pragma once

#include "domain/events/Event.h"
#include <string>
#include <variant>
#include <chrono>

namespace MusicTrainer::music::events {

/**
 * @brief Represents GUI state change events in the system
 * 
 * This class manages state change events for GUI components, providing
 * type-safe state updates with validation and tracking capabilities.
 * It supports multiple state types and includes source tracking for
 * debugging and validation purposes.
 */
class GuiStateEvent : public Event {
public:
    /**
     * @brief Types of GUI state changes supported by the system
     */
    enum class StateType {
        VIEWPORT_CHANGE,        ///< Changes to viewport position, zoom, or expansion
        SCORE_DISPLAY_CHANGE,  ///< Changes to score display options
        SELECTION_CHANGE,      ///< Changes to note or measure selection
        PLAYBACK_STATE_CHANGE, ///< Changes to playback status
        MIDI_DEVICE_CHANGE     ///< Changes to MIDI device configuration
    };

    /**
     * @brief Represents viewport state information
     */
    struct ViewportState {
        float x;                       ///< Horizontal scroll position
        float y;                       ///< Vertical scroll position
        float zoomLevel;              ///< Current zoom level
        bool preserveOctaveExpansion; ///< Whether to maintain expanded octave view
    };

    /**
     * @brief Represents score display options
     */
    struct ScoreDisplayState {
        bool showMeasureNumbers; ///< Whether to display measure numbers
        bool showKeySignature;   ///< Whether to display key signature
        bool showVoiceLabels;    ///< Whether to display voice labels
        int fontSize;            ///< Current font size for score text
    };

    /**
     * @brief Represents the current selection in the score
     */
    struct SelectionState {
        int voiceIndex;    ///< Selected voice index (-1 for none)
        int measureIndex;  ///< Selected measure index (-1 for none)
        int noteIndex;     ///< Selected note index (-1 for none)
    };

    /**
     * @brief Represents playback state information
     */
    struct PlaybackState {
        bool isPlaying;         ///< Whether playback is active
        bool isRecording;       ///< Whether recording is active
        int tempo;             ///< Current tempo in BPM
        bool metronomeEnabled; ///< Whether metronome is enabled
    };

    /**
     * @brief Represents MIDI device configuration
     */
    struct MidiDeviceState {
        int inputDeviceIndex;  ///< Selected input device (-1 for none)
        int outputDeviceIndex; ///< Selected output device (-1 for none)
        bool midiThrough;      ///< Whether MIDI through is enabled
        int latency;          ///< Current MIDI latency setting
    };

    /**
     * @brief Variant type containing all possible state types
     */
    using StateVariant = std::variant<
        ViewportState,
        ScoreDisplayState,
        SelectionState,
        PlaybackState,
        MidiDeviceState
    >;

    /**
     * @brief Creates a new GUI state event
     * 
     * @param type The type of state change
     * @param state The new state value
     * @param source Optional identifier for the source of the change
     * @return std::unique_ptr<GuiStateEvent> The created event
     */
    static std::unique_ptr<GuiStateEvent> create(
        StateType type,
        const StateVariant& state,
        const std::string& source = ""
    );

    /**
     * @brief Gets the type of state change
     * @return StateType The state change type
     */
    StateType getStateType() const { return m_stateType; }

    /**
     * @brief Gets the new state value
     * @return const StateVariant& The state value
     */
    const StateVariant& getState() const { return m_state; }

    /**
     * @brief Gets the source of the state change
     * @return const std::string& The source identifier
     */
    const std::string& getSource() const { return m_source; }

    /**
     * @brief Gets a human-readable description of the state change
     * 
     * The description includes the state type, source (if any), and
     * relevant details about the state change.
     *
     * @return std::string The event description
     */
    std::string getDescription() const override;

    /**
     * @brief Creates a deep copy of the event
     * @return std::unique_ptr<Event> A new copy of the event
     */
    std::unique_ptr<Event> clone() const override;

    /**
     * @brief Gets the type of the event
     * @return std::string The event type
     */
    std::string getType() const override { return "GuiStateEvent"; }
    
    /**
     * @brief Applies the event to the target score
     * @param target The target score
     */
    void apply(MusicTrainer::music::Score& target) const override {
        // GUI state events don't modify the score directly
    }
    
    /**
     * @brief Gets the timestamp of the event
     * @return std::chrono::system_clock::time_point The event timestamp
     */
    std::chrono::system_clock::time_point getTimestamp() const override {
        return m_timestamp;
    }

private:
    GuiStateEvent(StateType type, const StateVariant& state, const std::string& source);

    StateType m_stateType;    ///< Type of state change
    StateVariant m_state;     ///< New state value
    std::string m_source;     ///< Source of the change
    std::chrono::system_clock::time_point m_timestamp{std::chrono::system_clock::now()};
};

} // namespace MusicTrainer::music::events