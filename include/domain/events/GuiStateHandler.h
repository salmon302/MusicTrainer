#pragma once

#include "domain/events/EventHandler.h"
#include "domain/events/GuiStateEvent.h"
#include <functional>
#include <map>
#include <vector>
#include <mutex>
#include <optional>

namespace MusicTrainer::music::events {

/**
 * @brief Handles GUI state events and manages state subscriptions
 */
class GuiStateHandler : public EventHandler {
public:
    static std::shared_ptr<GuiStateHandler> create();
    ~GuiStateHandler() override = default;

    void handleEvent(const Event& event) override;

    /**
     * @brief Subscribe to state changes of a specific type
     * @param type The type of state to subscribe to
     * @param callback The function to call when the state changes
     */
    void subscribeToState(GuiStateEvent::StateType type, 
                         std::function<void(const GuiStateEvent::StateVariant&)> callback);

    /**
     * @brief Get the last known state of a specific type
     * @param type The type of state to retrieve
     * @return The last known state, or std::nullopt if not available
     */
    template<typename T>
    std::optional<T> getLastState(GuiStateEvent::StateType type) const;

private:
    GuiStateHandler() = default;

    void notifySubscribers(GuiStateEvent::StateType type, const GuiStateEvent::StateVariant& state);

    std::map<GuiStateEvent::StateType, std::vector<std::function<void(const GuiStateEvent::StateVariant&)>>> m_subscribers;
    std::map<GuiStateEvent::StateType, GuiStateEvent::StateVariant> m_lastStates;
    mutable std::mutex m_mutex;
};