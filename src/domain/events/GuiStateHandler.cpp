#include "domain/events/GuiStateHandler.h"
#include <algorithm>

namespace MusicTrainer::music::events {

std::shared_ptr<GuiStateHandler> GuiStateHandler::create() {
    return std::shared_ptr<GuiStateHandler>(new GuiStateHandler());
}

void GuiStateHandler::handleEvent(const Event& event) {
    const auto* stateEvent = dynamic_cast<const GuiStateEvent*>(&event);
    if (!stateEvent) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Update last known state
    m_lastStates[stateEvent->getStateType()] = stateEvent->getState();
    
    // Notify subscribers
    notifySubscribers(stateEvent->getStateType(), stateEvent->getState());
}

void GuiStateHandler::subscribeToState(GuiStateEvent::StateType type,
                                     std::function<void(const GuiStateEvent::StateVariant&)> callback) {
    if (!callback) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subscribers[type].push_back(std::move(callback));
    
    // Send current state to new subscriber if available
    auto it = m_lastStates.find(type);
    if (it != m_lastStates.end()) {
        callback(it->second);
    }
}

void GuiStateHandler::notifySubscribers(GuiStateEvent::StateType type,
                                      const GuiStateEvent::StateVariant& state) {
    auto it = m_subscribers.find(type);
    if (it == m_subscribers.end()) return;
    
    for (const auto& callback : it->second) {
        callback(state);
    }
}

template<typename T>
std::optional<T> GuiStateHandler::getLastState(GuiStateEvent::StateType type) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_lastStates.find(type);
    if (it == m_lastStates.end()) {
        return std::nullopt;
    }
    
    if (const T* state = std::get_if<T>(&it->second)) {
        return *state;
    }
    
    return std::nullopt;
}

void GuiStateHandler::clearState() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_lastStates.clear();
    m_subscribers.clear();
}

// Explicit template instantiations for the types we support
template std::optional<GuiStateEvent::ViewportState> 
GuiStateHandler::getLastState<GuiStateEvent::ViewportState>(GuiStateEvent::StateType) const;

template std::optional<GuiStateEvent::ScoreDisplayState> 
GuiStateHandler::getLastState<GuiStateEvent::ScoreDisplayState>(GuiStateEvent::StateType) const;

template std::optional<GuiStateEvent::SelectionState> 
GuiStateHandler::getLastState<GuiStateEvent::SelectionState>(GuiStateEvent::StateType) const;

template std::optional<GuiStateEvent::PlaybackState> 
GuiStateHandler::getLastState<GuiStateEvent::PlaybackState>(GuiStateEvent::StateType) const;

template std::optional<GuiStateEvent::MidiDeviceState> 
GuiStateHandler::getLastState<GuiStateEvent::MidiDeviceState>(GuiStateEvent::StateType) const;

} // namespace MusicTrainer::music::events