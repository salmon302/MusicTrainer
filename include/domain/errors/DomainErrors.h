#pragma once
#include "ErrorBase.h"

namespace MusicTrainer {

// Remove the redefinition of DomainError and use the one from ErrorBase.h
// Using more specific error types that derive from DomainError

// MIDI-related errors
class MidiError : public SystemError {
public:
	explicit MidiError(const std::string& message,
					  const ErrorContext& context = ErrorContext())
		: SystemError(message, "MidiError", context) {}
};

// Validation-related errors
class ValidationError : public DomainError {
public:
	explicit ValidationError(const std::string& message,
						   const ErrorContext& context = ErrorContext())
		: DomainError(message, "ValidationError", context) {}
};

// State management errors
class StateError : public DomainError {
public:
	explicit StateError(const std::string& message,
					   const ErrorContext& context = ErrorContext())
		: DomainError(message, "StateError", context) {}
};

// Plugin-related errors
class PluginError : public SystemError {
public:
	explicit PluginError(const std::string& message,
						const ErrorContext& context = ErrorContext())
		: SystemError(message, "PluginError", context) {}
};

// Exercise generation errors
class ExerciseGenerationError : public DomainError {
public:
	explicit ExerciseGenerationError(const std::string& message,
								   const ErrorContext& context = ErrorContext())
		: DomainError(message, "ExerciseGenerationError", context) {}
};

// Repository errors
class RepositoryError : public SystemError {
public:
	explicit RepositoryError(const std::string& message,
						   const ErrorContext& context = ErrorContext())
		: SystemError(message, "RepositoryError", context) {}
};

} // namespace MusicTrainer