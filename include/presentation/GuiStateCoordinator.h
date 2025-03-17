#pragma once

#include "domain/events/GuiStateEvent.h"
#include "domain/events/EventBus.h"
#include <memory>
#include <functional>
#include <optional>

namespace MusicTrainer::presentation {

/**
 * @brief Coordinates GUI state changes and synchronization
 * 
 * This class manages the synchronization of GUI state across components using
 * an event-based architecture. It handles subscription to state changes,
 * publishes state change events, and provides state recovery mechanisms.
 *
 * The coordinator implements an update coalescing pattern for performance
 * optimization and includes state validation with automatic recovery.
 *
 * @see GuiStateEvent
 * @see EventBus
 */
class GuiStateCoordinator {
public:
    /**
     * @brief Creates a new GuiStateCoordinator instance
     * @param eventBus The event bus to use for state change events
     * @return A shared pointer to the new coordinator
     * @throws InvalidArgumentError if eventBus is null
     */
    static std::shared_ptr<GuiStateCoordinator> create(std::shared_ptr<music::events::EventBus> eventBus);
    ~GuiStateCoordinator();

    /**
     * @brief Starts the coordinator
     * 
     * Initializes event handlers and begins processing state changes.
     * Must be called before using the coordinator.
     */
    void start();

    /**
     * @brief Stops the coordinator
     * 
     * Cleans up event handlers and stops processing state changes.
     * Should be called before destruction for proper cleanup.
     */
    void stop();

    /**
     * @brief Subscribes to state changes of a specific type
     * 
     * The callback will be invoked whenever a state change of the specified type occurs.
     * New subscribers immediately receive the current state if available.
     *
     * @tparam T The type of state to subscribe to (ViewportState, ScoreDisplayState, etc.)
     * @param type The type of state to subscribe to
     * @param callback The function to call when the state changes
     * @throws InvalidArgumentError if callback is null
     */
    template<typename T>
    void subscribeToState(music::events::GuiStateEvent::StateType type, 
                         std::function<void(const T&)> callback);

    /**
     * @brief Gets the last known state of a specific type
     * 
     * @tparam T The type of state to retrieve
     * @param type The type of state to retrieve
     * @return The last known state, or std::nullopt if not available
     */
    template<typename T>
    std::optional<T> getLastState(music::events::GuiStateEvent::StateType type) const;

    /**
     * @brief Updates viewport state and notifies subscribers
     * 
     * @param state The new viewport state
     */
    void updateViewportState(const music::events::GuiStateEvent::ViewportState& state);

    /**
     * @brief Updates score display state and notifies subscribers
     * 
     * @param state The new score display state
     */
    void updateScoreDisplayState(const music::events::GuiStateEvent::ScoreDisplayState& state);

    /**
     * @brief Updates selection state and notifies subscribers
     * 
     * @param state The new selection state
     */
    void updateSelectionState(const music::events::GuiStateEvent::SelectionState& state);

    /**
     * @brief Updates playback state and notifies subscribers
     * 
     * @param state The new playback state
     */
    void updatePlaybackState(const music::events::GuiStateEvent::PlaybackState& state);

    /**
     * @brief Updates MIDI device state and notifies subscribers
     * 
     * @param state The new MIDI device state
     */
    void updateMidiDeviceState(const music::events::GuiStateEvent::MidiDeviceState& state);

private:
    GuiStateCoordinator(std::shared_ptr<music::events::EventBus> eventBus);

    /**
     * @brief Internal method to update state and publish events
     * 
     * @tparam T The type of state to update
     * @param type The type of state being updated
     * @param state The new state
     * @param source Optional source identifier for the state change
     */
    template<typename T>
    void updateState(music::events::GuiStateEvent::StateType type,
                    const T& state,
                    const std::string& source = "");

    std::shared_ptr<music::events::EventBus> m_eventBus;
    std::shared_ptr<music::events::GuiStateHandler> m_stateHandler;
    bool m_isStarted{false};
};

} // namespace MusicTrainer::presentation