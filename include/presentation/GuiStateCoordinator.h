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
 * an event-based architecture. It handles subscription to state changes and
 * publishes state change events.
 */
class GuiStateCoordinator {
public:
    /**
     * @brief Creates a new GuiStateCoordinator instance
     * @param eventBus The event bus to use for state change events
     * @return A shared pointer to the new coordinator
     */
    static std::shared_ptr<GuiStateCoordinator> create(std::shared_ptr<music::events::EventBus> eventBus);
    ~GuiStateCoordinator();

    /**
     * @brief Starts the coordinator
     * 
     * Initializes event handlers and begins processing state changes.
     */
    void start();

    /**
     * @brief Stops the coordinator
     * 
     * Cleans up event handlers and stops processing state changes.
     */
    void stop();

    /**
     * @brief Subscribes to state changes of a specific type
     * @param type The type of state to subscribe to
     * @param callback The function to call when the state changes
     */
    template<typename T>
    void subscribeToState(music::events::GuiStateEvent::StateType type, 
                         std::function<void(const T&)> callback);

    /**
     * @brief Gets the last known state of a specific type
     * @param type The type of state to retrieve
     * @return The last known state, or std::nullopt if not available
     */
    template<typename T>
    std::optional<T> getLastState(music::events::GuiStateEvent::StateType type) const;

    // Convenience methods for updating specific states
    void updateViewportState(const music::events::GuiStateEvent::ViewportState& state);
    void updateScoreDisplayState(const music::events::GuiStateEvent::ScoreDisplayState& state);
    void updateSelectionState(const music::events::GuiStateEvent::SelectionState& state);
    void updatePlaybackState(const music::events::GuiStateEvent::PlaybackState& state);
    void updateMidiDeviceState(const music::events::GuiStateEvent::MidiDeviceState& state);

private:
    GuiStateCoordinator(std::shared_ptr<music::events::EventBus> eventBus);

    template<typename T>
    void updateState(music::events::GuiStateEvent::StateType type,
                    const T& state,
                    const std::string& source = "");

    std::shared_ptr<music::events::EventBus> m_eventBus;
    std::shared_ptr<music::events::GuiStateHandler> m_stateHandler;
    bool m_isStarted{false};
};