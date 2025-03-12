#include "presentation/GuiStateCoordinator.h"
#include "domain/errors/DomainErrors.h"

namespace MusicTrainer::presentation {

std::shared_ptr<GuiStateCoordinator> GuiStateCoordinator::create(std::shared_ptr<music::events::EventBus> eventBus) {
    if (!eventBus) {
        throw music::errors::InvalidArgumentError("EventBus cannot be null");
    }
    return std::shared_ptr<GuiStateCoordinator>(new GuiStateCoordinator(eventBus));
}

GuiStateCoordinator::GuiStateCoordinator(std::shared_ptr<music::events::EventBus> eventBus)
    : m_eventBus(eventBus)
    , m_stateHandler(music::events::GuiStateHandler::create())
{
}

GuiStateCoordinator::~GuiStateCoordinator() {
    stop();
}

void GuiStateCoordinator::start() {
    if (m_isStarted) return;
    
    m_eventBus->registerHandler(m_stateHandler);
    m_isStarted = true;
}

void GuiStateCoordinator::stop() {
    if (!m_isStarted) return;
    
    m_eventBus->unregisterHandler(m_stateHandler);
    m_isStarted = false;
}

void GuiStateCoordinator::updateViewportState(const music::events::GuiStateEvent::ViewportState& state) {
    updateState(music::events::GuiStateEvent::StateType::VIEWPORT_CHANGE, state);
}

void GuiStateCoordinator::updateScoreDisplayState(const music::events::GuiStateEvent::ScoreDisplayState& state) {
    updateState(music::events::GuiStateEvent::StateType::SCORE_DISPLAY_CHANGE, state);
}

void GuiStateCoordinator::updateSelectionState(const music::events::GuiStateEvent::SelectionState& state) {
    updateState(music::events::GuiStateEvent::StateType::SELECTION_CHANGE, state);
}

void GuiStateCoordinator::updatePlaybackState(const music::events::GuiStateEvent::PlaybackState& state) {
    updateState(music::events::GuiStateEvent::StateType::PLAYBACK_STATE_CHANGE, state);
}

void GuiStateCoordinator::updateMidiDeviceState(const music::events::GuiStateEvent::MidiDeviceState& state) {
    updateState(music::events::GuiStateEvent::StateType::MIDI_DEVICE_CHANGE, state);
}

template<typename T>
void GuiStateCoordinator::subscribeToState(music::events::GuiStateEvent::StateType type,
                                         std::function<void(const T&)> callback) {
    if (!callback) {
        throw music::errors::InvalidArgumentError("Callback cannot be null");
    }
    
    auto wrappedCallback = [callback](const music::events::GuiStateEvent::StateVariant& state) {
        if (const T* typedState = std::get_if<T>(&state)) {
            callback(*typedState);
        }
    };
    
    m_stateHandler->subscribeToState(type, wrappedCallback);
}

template<typename T>
std::optional<T> GuiStateCoordinator::getLastState(music::events::GuiStateEvent::StateType type) const {
    return m_stateHandler->getLastState<T>(type);
}

template<typename T>
void GuiStateCoordinator::updateState(music::events::GuiStateEvent::StateType type,
                                    const T& state,
                                    const std::string& source) {
    auto event = music::events::GuiStateEvent::create(type, state, source);
    m_eventBus->publishAsync(std::move(event));
}

// Explicit template instantiations
template void GuiStateCoordinator::subscribeToState<music::events::GuiStateEvent::ViewportState>(
    music::events::GuiStateEvent::StateType, std::function<void(const music::events::GuiStateEvent::ViewportState&)>);
template void GuiStateCoordinator::subscribeToState<music::events::GuiStateEvent::ScoreDisplayState>(
    music::events::GuiStateEvent::StateType, std::function<void(const music::events::GuiStateEvent::ScoreDisplayState&)>);
template void GuiStateCoordinator::subscribeToState<music::events::GuiStateEvent::SelectionState>(
    music::events::GuiStateEvent::StateType, std::function<void(const music::events::GuiStateEvent::SelectionState&)>);
template void GuiStateCoordinator::subscribeToState<music::events::GuiStateEvent::PlaybackState>(
    music::events::GuiStateEvent::StateType, std::function<void(const music::events::GuiStateEvent::PlaybackState&)>);
template void GuiStateCoordinator::subscribeToState<music::events::GuiStateEvent::MidiDeviceState>(
    music::events::GuiStateEvent::StateType, std::function<void(const music::events::GuiStateEvent::MidiDeviceState&)>);

template std::optional<music::events::GuiStateEvent::ViewportState> 
GuiStateCoordinator::getLastState<music::events::GuiStateEvent::ViewportState>(music::events::GuiStateEvent::StateType) const;
template std::optional<music::events::GuiStateEvent::ScoreDisplayState>
GuiStateCoordinator::getLastState<music::events::GuiStateEvent::ScoreDisplayState>(music::events::GuiStateEvent::StateType) const;
template std::optional<music::events::GuiStateEvent::SelectionState>
GuiStateCoordinator::getLastState<music::events::GuiStateEvent::SelectionState>(music::events::GuiStateEvent::StateType) const;
template std::optional<music::events::GuiStateEvent::PlaybackState>
GuiStateCoordinator::getLastState<music::events::GuiStateEvent::PlaybackState>(music::events::GuiStateEvent::StateType) const;
template std::optional<music::events::GuiStateEvent::MidiDeviceState>
GuiStateCoordinator::getLastState<music::events::GuiStateEvent::MidiDeviceState>(music::events::GuiStateEvent::StateType) const;

template void GuiStateCoordinator::updateState<music::events::GuiStateEvent::ViewportState>(
    music::events::GuiStateEvent::StateType, const music::events::GuiStateEvent::ViewportState&, const std::string&);
template void GuiStateCoordinator::updateState<music::events::GuiStateEvent::ScoreDisplayState>(
    music::events::GuiStateEvent::StateType, const music::events::GuiStateEvent::ScoreDisplayState&, const std::string&);
template void GuiStateCoordinator::updateState<music::events::GuiStateEvent::SelectionState>(
    music::events::GuiStateEvent::StateType, const music::events::GuiStateEvent::SelectionState&, const std::string&);
template void GuiStateCoordinator::updateState<music::events::GuiStateEvent::PlaybackState>(
    music::events::GuiStateEvent::StateType, const music::events::GuiStateEvent::PlaybackState&, const std::string&);
template void GuiStateCoordinator::updateState<music::events::GuiStateEvent::MidiDeviceState>(
    music::events::GuiStateEvent::StateType, const music::events::GuiStateEvent::MidiDeviceState&, const std::string&);

} // namespace MusicTrainer::presentation