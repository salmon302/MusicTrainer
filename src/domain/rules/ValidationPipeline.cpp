#include "domain/rules/ValidationPipeline.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "utils/TrackedLock.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>
#include <future>

namespace {
	void updateMetrics(music::rules::ValidationPipeline::ValidationMetrics& metrics, bool isHit) {
		if (isHit) {
			metrics.cacheHits++;
		} else {
			metrics.cacheMisses++;
		}
		metrics.ruleExecutions++;
		metrics.cacheHitRate = static_cast<double>(metrics.cacheHits) / 
			(metrics.cacheHits + metrics.cacheMisses);
	}
}

namespace music::rules {

std::unique_ptr<ValidationPipeline> ValidationPipeline::create() {
	return std::unique_ptr<ValidationPipeline>(new ValidationPipeline());
}

void ValidationPipeline::addRule(std::unique_ptr<Rule> rule, std::vector<std::string> dependencies, int priority) {
	::utils::TrackedUniqueLock lock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
	rules.emplace_back(std::move(rule), std::move(dependencies), priority);
	compiled = false;
}

void ValidationPipeline::clearRuleCache() {
	::utils::TrackedUniqueLock lock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
	ruleCache.clear();
	for (auto& metadata : rules) {
		metadata.lastExecutionTime = std::chrono::microseconds{0};
		metadata.lastValidatedMeasure = 0;
	}
	metrics = ValidationMetrics{};
}

bool ValidationPipeline::evaluateRule(RuleMetadata& metadata, const Score& score, size_t measureIndex) {
	try {
		// Get snapshot without holding locks
		auto scoreSnapshot = score.createSnapshot();
		
		// Execute rule without locks
		auto future = std::async(std::launch::async, [&metadata, scoreSnapshot, measureIndex]() {
			Score tempScore(scoreSnapshot);
			return metadata.incremental ? 
				static_cast<IncrementalRule*>(metadata.rule.get())->evaluateIncremental(tempScore, measureIndex, measureIndex + 1) :
				metadata.rule->evaluate(tempScore);
		});
		
		// Wait for result with timeout
		if (future.wait_for(ruleTimeout) == std::future_status::timeout) {
			return false;
		}
		bool ruleResult = future.get();
		
		// Update metrics under single lock
		{
			::utils::TrackedUniqueLock metricsLock(metrics_mutex_, "ValidationPipeline::metrics_mutex_", ::utils::LockLevel::METRICS);
			metadata.lastExecutionTime = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch());
			metadata.lastValidatedMeasure = measureIndex;
			metrics.ruleExecutions++;
			metrics.totalExecutionTime += metadata.lastExecutionTime;
			metrics.maxExecutionTime = std::max(metrics.maxExecutionTime, metadata.lastExecutionTime);
			metrics.avgExecutionTime = metrics.totalExecutionTime / metrics.ruleExecutions;
			metrics.ruleTimings.push_back({metadata.rule->getName(), metadata.lastExecutionTime});
		}
		
		return ruleResult;
	} catch (const std::exception& e) {
		std::cerr << "[Pipeline::evaluateRule] Error: " << e.what() << std::endl;
		throw;
	}
}



bool ValidationPipeline::validate(const Score& score) {
	try {
		// Get snapshot and rules without holding locks
		auto scoreSnapshot = score.createSnapshot();
		std::vector<std::reference_wrapper<RuleMetadata>> ruleRefs;
		bool needsCompilation = false;
		
		{
			::utils::TrackedSharedMutexLock validationLock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
			needsCompilation = !compiled;
			if (!needsCompilation) {
				ruleRefs.reserve(rules.size());
				for (auto& rule : rules) {
					ruleRefs.emplace_back(std::ref(rule));
				}
			}
		}
		
		if (needsCompilation) {
			compileRules();
			::utils::TrackedSharedMutexLock validationLock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
			ruleRefs.reserve(rules.size());
			for (auto& rule : rules) {
				ruleRefs.emplace_back(std::ref(rule));
			}
		}
		
		// Evaluate rules without holding locks
		bool allValid = true;
		std::vector<std::string> pendingViolations;
		
		for (auto& metadataRef : ruleRefs) {
			if (!evaluateRule(metadataRef.get(), score, 0)) {
				allValid = false;
				pendingViolations.push_back(metadataRef.get().rule->getViolationDescription());
			}
		}
		
		// Update state under single lock
		if (!pendingViolations.empty()) {
			::utils::TrackedUniqueLock validationLock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
			violations = std::move(pendingViolations);
			metrics.violationsCount += violations.size();
		}
		
		return allValid;
	} catch (const std::exception& e) {
		std::cerr << "[Pipeline::validate] Error: " << e.what() << std::endl;
		throw;
	}
}





bool ValidationPipeline::validateIncremental(const Score& score, size_t startMeasure) {
	std::cout << "[Pipeline] Starting incremental validation from measure " << startMeasure << "..." << std::endl;
	
	// Get snapshot before acquiring any locks
	auto scoreSnapshot = score.createSnapshot();
	
	std::vector<std::reference_wrapper<RuleMetadata>> ruleRefs;
	bool needsCompilation = false;
	{
		::utils::TrackedSharedMutexLock validationLock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
		needsCompilation = !compiled;
		if (!needsCompilation) {
			ruleRefs.reserve(rules.size());
			for (auto& rule : rules) {
				ruleRefs.push_back(std::ref(rule));
			}
		}
	}
	
	// Compile rules if needed
	if (needsCompilation) {
		compileRules();
		::utils::TrackedSharedMutexLock validationLock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
		ruleRefs.reserve(rules.size());
		for (auto& rule : rules) {
			ruleRefs.push_back(std::ref(rule));
		}
	}
	
	bool isValid = true;
	std::vector<std::pair<std::string, size_t>> pendingViolations;
	
	// Evaluate rules without holding any locks
	for (auto& metadataRef : ruleRefs) {
		auto& metadata = metadataRef.get();
		bool ruleResult = evaluateRule(metadata, score, startMeasure);
		if (!ruleResult) {
			auto violation = metadata.rule->getViolationDescription();
			if (!violation.empty()) {
				pendingViolations.emplace_back(violation, metadata.priority);
				isValid = false;
			}
		}
	}

	if (!pendingViolations.empty()) {
		// Update metrics first (level 5)
		{
			::utils::TrackedUniqueLock metricsLock(metrics_mutex_, "ValidationPipeline::metrics_mutex_", ::utils::LockLevel::METRICS);
			metrics.violationsCount += pendingViolations.size();
		}
		
		// Then update validation state (level 4)
		{
			::utils::TrackedUniqueLock validationLock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
			violations.clear();
			feedbackItems.clear();
			for (const auto& [violation, priority] : pendingViolations) {
				violations.push_back(violation);
				FeedbackLevel level = priority > 5 ? FeedbackLevel::ERROR : FeedbackLevel::WARNING;
				ValidationFeedback feedback{violation, level, startMeasure, priority};
				feedbackItems.push_back(feedback);
			}
		}
	}
	
	std::cout << "[Pipeline] Incremental validation complete. Result: " << (isValid ? "true" : "false") << std::endl;
	return isValid;
}



void ValidationPipeline::sortRulesByDependencies(std::vector<RuleMetadata>& rulesCopy, std::vector<size_t>& order) {
	std::vector<bool> visited(rulesCopy.size(), false);
	std::vector<bool> inStack(rulesCopy.size(), false);
	order.clear();
	
	std::function<void(size_t)> visit = [&](size_t i) {
		if (inStack[i]) {
			throw std::runtime_error("Circular dependency detected in rules");
		}
		if (visited[i]) return;
		
		inStack[i] = true;
		visited[i] = true;
		
		for (const auto& dep : rulesCopy[i].dependencies) {
			for (size_t j = 0; j < rulesCopy.size(); ++j) {
				if (rulesCopy[j].rule->getName() == dep) {
					visit(j);
					break;
				}
			}
		}
		
		inStack[i] = false;
		order.push_back(i);
	};
	
	for (size_t i = 0; i < rulesCopy.size(); ++i) {
		if (!visited[i]) {
			visit(i);
		}
	}
	
	std::reverse(order.begin(), order.end());
}

void ValidationPipeline::compileRules() {
	std::vector<RuleMetadata> rulesCopy;
	{
		::utils::TrackedSharedMutexLock lock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
		rulesCopy = rules;
	}
	
	if (rulesCopy.empty()) {
		::utils::TrackedUniqueLock lock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
		compiled = true;
		return;
	}
	
	// Sort rules without holding locks
	std::vector<size_t> newOrder;
	sortRulesByDependencies(rulesCopy, newOrder);
	
	// Update state under single lock
	{
		::utils::TrackedUniqueLock lock(mutex_, "ValidationPipeline::mutex_", ::utils::LockLevel::VALIDATION);
		evaluationOrder = std::move(newOrder);
		rules = std::move(rulesCopy);
		compiled = true;
	}
}


std::vector<std::string> ValidationPipeline::getViolations() const {
	return violations;
}

void ValidationPipeline::clearViolations() {
	violations.clear();
}

} // namespace music::rules
