#ifndef MUSICTRAINERV3_ERRORHANDLING_H
#define MUSICTRAINERV3_ERRORHANDLING_H

#include "domain/errors/ErrorHandler.h"
#include "domain/errors/DefaultErrorHandlers.h"

// Single, synchronous error handling macro
#define HANDLE_ERROR(error) \
	{ \
		auto& handler = MusicTrainer::ErrorHandler::getInstance(); \
		handler.handleError(error).wait(); \
	}

// Configure error handlers for tests
inline void configureTestErrorHandlers() {
	auto& handler = MusicTrainer::ErrorHandler::getInstance();
	handler.clearHandlers();  // Clear any existing handlers
	MusicTrainer::configureDefaultErrorHandlers();
}

#endif // MUSICTRAINERV3_ERRORHANDLING_H