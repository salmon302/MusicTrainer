#pragma once

#include "domain/events/Event.h"
#include <memory>

namespace MusicTrainer::music::events {

class EventHandler {
public:
    virtual ~EventHandler() = default;
    
    // Handle an incoming event
    virtual void handleEvent(const Event& event) = 0;
};

} // namespace MusicTrainer::music::events