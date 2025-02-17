#include "domain/crossdomain/CrossDomainValidator.h"
#include "domain/rules/ValidationPipeline.h"
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/ParallelOctavesRule.h"
#include "domain/rules/VoiceLeadingRule.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <sstream>
#include <iostream>
#include <future>

namespace music {
namespace crossdomain {

CrossDomainValidator::CrossDomainValidator(rules::ValidationPipeline& pipeline, ports::ScoreRepository& repository)
	: pipeline(pipeline), repository(repository) {
	// Register all rules with appropriate priorities
	pipeline.addRule(rules::ParallelFifthsRule::create(), {}, 10);
	pipeline.addRule(rules::ParallelOctavesRule::create(), {}, 10);
	pipeline.addRule(rules::VoiceLeadingRule::create(), {"Parallel Fifths Rule", "Parallel Octaves Rule"}, 5);
}

ValidationResult CrossDomainValidator::validate(const Score& score) {
	bool isValid = true;
	std::stringstream message;

	try {
		// Get score snapshot first to avoid holding locks while validating
		auto scoreSnapshot = score.createSnapshot();
		Score tempScore(scoreSnapshot);
		
		// Check pipeline validation (ValidationPipeline handles its own locking)
		isValid = pipeline.validate(tempScore);
		if (!isValid) {
			message << "Validation pipeline failed.\n";
			auto feedback = pipeline.getFeedback();
			for (const auto& item : feedback) {
				message << "- " << item.message << "\n";
			}
			return {message.str(), isValid};
		}

		// Check repository version (Repository handles its own locking)
		auto scoreName = tempScore.getName();
		try {
			auto existingScore = repository.load(scoreName);
			if (existingScore && existingScore->getVersion() > tempScore.getVersion()) {
				isValid = false;
				message << "Score version conflict detected.\n";
			}
		} catch (const std::exception&) {
			// Score doesn't exist in repository - this is fine for new scores
		}
	}
	catch (const std::exception& e) {
		std::string errorMsg = "Cross-domain validation failed: " + std::string(e.what());
		auto error = MusicTrainer::ValidationError(errorMsg);
		// ErrorHandler handles its own locking
		MusicTrainer::ErrorHandler::getInstance().handleError(error).wait();
		
		message << "Error during validation: " << e.what() << "\n";
		isValid = false;
	}

	return {message.str(), isValid};
}

} // namespace crossdomain
} // namespace music

