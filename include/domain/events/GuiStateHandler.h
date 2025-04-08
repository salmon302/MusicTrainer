#pragma once

#include "domain/events/EventHandler.h"
#include "domain/events/GuiStateEvent.h"
#include <functional>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <memory>

namespace MusicTrainer::music::events {

/**
 * @brief Handles GUI state events and manages state subscriptions
 *
 * This class is responsible for managing state updates, validating changes,
 * and providing recovery mechanisms for invalid states. It maintains the last
 * known good state for each state type and supports automatic state recovery.
 *
 * Thread safety is ensured through proper synchronization of state access
 * and subscription management.
 */
class GuiStateHandler : public EventHandler, 
                       public std::enable_shared_from_this<GuiStateHandler> {
public:
    /**
     * @brief Creates a new GuiStateHandler instance
     * @return std::shared_ptr<GuiStateHandler> The created handler
     */
    static std::shared_ptr<GuiStateHandler> create();
    ~GuiStateHandler() override = default;

    /**
     * @brief Handles incoming GUI state events
     * 
     * Validates and processes state change events, maintaining the last known
     * good state and notifying subscribers. Implements automatic state recovery
     * for invalid states.
     *
     * @param event The event to handle
     */
    void handleEvent(const Event& event) override;

    /**
     * @brief Subscribe to state changes of a specific type
     * 
     * Subscribers are immediately notified of the current state if available.
     * Multiple subscribers for the same state type are supported and notifications
     * are thread-safe.
     *
     * @param type The type of state to subscribe to
     * @param callback The function to call when the state changes
     */
    void subscribeToState(GuiStateEvent::StateType type, 
                         std::function<void(const GuiStateEvent::StateVariant&)> callback);

    /**
     * @brief Gets the last known state of a specific type
     * 
     * Thread-safe access to the most recent valid state. Can return nullopt
     * if no valid state exists or if the state has been cleared.
     *
     * @tparam T The type of state to retrieve
     * @param type The type of state to retrieve
     * @return std::optional<T> The last known state, or nullopt if not available
     */
    template<typename T>
    std::optional<T> getLastState(GuiStateEvent::StateType type) const;

    /**
     * @brief Clear all stored states and subscriptions
     *
     * Resets all state history and removes all subscribers. Used during
     * error recovery or when a full reset is needed.
     */
    void clearState();

protected:
    // Protected constructor allows make_shared to work
    GuiStateHandler() = default;

private:
    /**
     * @brief Notify all subscribers of a state change
     * 
     * Thread-safe notification of all registered subscribers for a given
     * state type. Subscribers are notified in registration order.
     *
     * @param type The type of state that changed
     * @param state The new state value
     */
    void notifySubscribers(GuiStateEvent::StateType type, 
                          const GuiStateEvent::StateVariant& state);

    // Subscriber callbacks for each state type
    std::map<GuiStateEvent::StateType, 
             std::vector<std::function<void(const GuiStateEvent::StateVariant&)>>> m_subscribers;

    // Last known good state for each state type
    std::map<GuiStateEvent::StateType, GuiStateEvent::StateVariant> m_lastStates;

    // Mutex for thread-safe state access
    mutable std::mutex m_mutex;
};

} // namespace MusicTrainer::music::events