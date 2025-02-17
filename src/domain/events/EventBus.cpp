#include "domain/events/EventBus.h"

namespace music::events {

std::unique_ptr<EventBus> EventBus::create() {
	return std::unique_ptr<EventBus>(new EventBus());
}

} // namespace music::events



