#pragma once

#include "Event.h"
#include <string>

namespace music::events {

class EventHandler {

public:
	virtual ~EventHandler() = default;
	virtual void handle(const Event& event) = 0;
	virtual bool canHandle(const std::string& eventType) const = 0;
	virtual int getPriority() const { return 0; }  // Default priority
};

} // namespace music::events