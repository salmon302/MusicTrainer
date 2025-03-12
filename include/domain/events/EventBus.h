#pragma once

#include "domain/events/Event.h"
#include "domain/events/EventHandler.h"
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>

namespace MusicTrainer::music::events {

class EventBus {
public:
    static std::shared_ptr<EventBus> create();
    ~EventBus();

    // Start processing events
    void start();

    // Stop processing events
    void stop();

    // Publish an event asynchronously
    void publishAsync(std::unique_ptr<Event> event);

    // Register an event handler
    void registerHandler(std::shared_ptr<EventHandler> handler);

    // Unregister an event handler
    void unregisterHandler(std::shared_ptr<EventHandler> handler);

private:
    EventBus();
    
    void processEvents();
    void notifyHandlers(const Event& event);

    std::vector<std::shared_ptr<EventHandler>> m_handlers;
    std::queue<std::unique_ptr<Event>> m_eventQueue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_processingThread;
    std::atomic<bool> m_isRunning{false};
    std::mutex m_handlerMutex;
};

} // namespace MusicTrainer::music::events




