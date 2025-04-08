#pragma once

#include "domain/events/Event.h"
#include <memory>

namespace MusicTrainer::music::events {

class EventHandler {
public:
    virtual ~EventHandler() = default;
    
    // Handle an incoming event
    virtual void handleEvent(const Event& event) = 0;
    
    // Check if this handler can handle a given event type
    virtual bool canHandle(const Event& event) const { 
        return true; // Base implementation accepts all events
    }
    
    // Get the priority of this handler (higher priority handlers run first)
    virtual int getPriority() const { 
        return 0; // Default priority
    }
};

} // namespace MusicTrainer::music::events