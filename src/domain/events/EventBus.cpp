#include "domain/events/EventBus.h"
#include "domain/errors/DomainErrors.h"
#include <chrono>
#include <algorithm>

namespace MusicTrainer::music::events {

std::shared_ptr<EventBus> EventBus::create() {
    return std::shared_ptr<EventBus>(new EventBus());
}

EventBus::EventBus() = default;

EventBus::~EventBus() {
    stop();
}

void EventBus::start() {
    if (m_isRunning) return;
    
    m_isRunning = true;
    m_processingThread = std::thread(&EventBus::processEvents, this);
}

void EventBus::stop() {
    if (!m_isRunning) return;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning = false;
    }
    m_condition.notify_one();
    
    if (m_processingThread.joinable()) {
        m_processingThread.join();
    }
}

void EventBus::publishAsync(std::unique_ptr<Event> event) {
    if (!event) return;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_eventQueue.push(std::move(event));
    }
    m_condition.notify_one();
}

void EventBus::registerHandler(std::shared_ptr<EventHandler> handler) {
    if (!handler) return;
    
    std::lock_guard<std::mutex> lock(m_handlerMutex);
    if (std::find(m_handlers.begin(), m_handlers.end(), handler) == m_handlers.end()) {
        m_handlers.push_back(handler);
    }
}

void EventBus::unregisterHandler(std::shared_ptr<EventHandler> handler) {
    if (!handler) return;
    
    std::lock_guard<std::mutex> lock(m_handlerMutex);
    m_handlers.erase(
        std::remove(m_handlers.begin(), m_handlers.end(), handler),
        m_handlers.end()
    );
}

void EventBus::processEvents() {
    while (true) {
        std::unique_ptr<Event> event;
        
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] {
                return !m_isRunning || !m_eventQueue.empty();
            });
            
            if (!m_isRunning && m_eventQueue.empty()) {
                break;
            }
            
            if (!m_eventQueue.empty()) {
                event = std::move(m_eventQueue.front());
                m_eventQueue.pop();
            }
        }
        
        if (event) {
            notifyHandlers(*event);
        }
    }
}

void EventBus::notifyHandlers(const Event& event) {
    std::lock_guard<std::mutex> lock(m_handlerMutex);
    for (const auto& handler : m_handlers) {
        handler->handleEvent(event);
    }
}

} // namespace MusicTrainer::music::events



