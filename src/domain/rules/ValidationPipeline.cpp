#include "domain/rules/ValidationPipeline.h"
#include <algorithm>
#include <future>
#include <iostream>

using namespace std;

namespace music::rules {

std::unique_ptr<ValidationPipeline> ValidationPipeline::create() {
	return std::unique_ptr<ValidationPipeline>(new ValidationPipeline());
}

void ValidationPipeline::addRule(std::unique_ptr<Rule> rule, std::vector<std::string> dependencies, int priority) {
	rules.emplace_back(std::move(rule), std::move(dependencies), priority);
	compiled = false;
}

void ValidationPipeline::compileRules() {
	if (compiled) return;
	sortRulesByDependencies();
	compiled = true;
}

ValidationPipeline::ValidationPipeline() : metrics{} {
	// Initialize metrics with zero values
	metrics.totalExecutionTime = 0;
	metrics.maxExecutionTime = 0;
	metrics.avgExecutionTime = 0;
	metrics.ruleExecutions = 0;
	metrics.cacheHits = 0;
	metrics.cacheMisses = 0;
	metrics.cacheHitRate = 0.0;
	metrics.violationsCount = 0;
}

bool ValidationPipeline::validate(const Score& score) {
	if (!compiled) {
		std::cout << "[ValidationPipeline] Rules not compiled, compiling now..." << std::endl;
		compileRules();
	}
	violations.clear();
	bool allValid = true;
	
	std::cout << "[ValidationPipeline] Starting validation with " << rules.size() << " rules" << std::endl;
	for (auto& metadata : rules) {
		std::cout << "[ValidationPipeline] Evaluating rule: " << metadata.rule->getName() << std::endl;
		bool ruleResult = evaluateRule(metadata, score, 0);
		std::cout << "[ValidationPipeline] Rule result: " << (ruleResult ? "valid" : "invalid") << std::endl;
		if (!ruleResult) {
			std::cout << "[ValidationPipeline] Rule " << metadata.rule->getName() << " failed with violations:" << std::endl;
			for (const auto& violation : violations) {
				std::cout << "  - " << violation << std::endl;
			}
			allValid = false;
		}
	}
	
	std::cout << "[ValidationPipeline] Final result: " << (allValid ? "valid" : "invalid") << std::endl;
	std::cout << "[ValidationPipeline] Total violations: " << violations.size() << std::endl;
	std::cout << "[ValidationPipeline] Returning: " << (allValid ? "valid" : "invalid") << std::endl;
	return allValid;  // true = all rules passed, false = at least one rule failed
}

bool ValidationPipeline::validateIncremental(const Score& score, size_t startMeasure) {
	if (!compiled) {
		std::cout << "[ValidationPipeline::validateIncremental] Rules not compiled, compiling now..." << std::endl;
		compileRules();
	}
	bool valid = true;
	
	std::cout << "[ValidationPipeline::validateIncremental] Starting validation from measure " << startMeasure << std::endl;
	for (auto& metadata : rules) {
		// All rules should be evaluated in incremental validation
		std::cout << "[ValidationPipeline::validateIncremental] Evaluating rule: " << metadata.rule->getName() << std::endl;
		bool ruleResult = evaluateRule(metadata, score, startMeasure);
		std::cout << "[ValidationPipeline::validateIncremental] Rule result: " << (ruleResult ? "valid" : "invalid") << std::endl;
		if (!ruleResult) {
			std::cout << "[ValidationPipeline::validateIncremental] Rule " << metadata.rule->getName() << " failed with violations:" << std::endl;
			for (const auto& violation : violations) {
				std::cout << "  - " << violation << std::endl;
			}
			valid = false;
		}
	}
	
	std::cout << "[ValidationPipeline::validateIncremental] Final result: " << (valid ? "valid" : "invalid") << std::endl;
	std::cout << "[ValidationPipeline::validateIncremental] Total violations: " << violations.size() << std::endl;
	return valid;  // true = all rules passed, false = at least one rule failed
}

bool ValidationPipeline::evaluateRule(RuleMetadata& metadata, const Score& score, size_t measureIndex) {
	std::cout << "\n[ValidationPipeline::evaluateRule] Starting evaluation of " << metadata.rule->getName() << std::endl;
	
	// Create cache key
	CacheKey key{
		metadata.rule->getName(),
		measureIndex,
		score.getHash()
	};
	std::cout << "[ValidationPipeline::evaluateRule] Cache key: name=" << key.ruleName
			  << ", measure=" << key.measureIndex
			  << ", hash=" << key.scoreHash << std::endl;
	
	// Check cache first
	auto cacheIt = ruleCache.find(key);
	if (cacheIt != ruleCache.end()) {
		metrics.cacheHits++;
		metrics.ruleExecutions++;  // Count both cache hits and misses
		std::cout << "[ValidationPipeline::evaluateRule] Cache hit! Result: "
				  << (cacheIt->second ? "valid" : "invalid") << std::endl;
		if (!cacheIt->second) {
			std::string violation = metadata.rule->getName() + ": " + metadata.rule->getViolationDescription();
			violations.push_back(violation);
			std::cout << "[ValidationPipeline::evaluateRule] Restored cached violation: " << violation << std::endl;
		}
		return cacheIt->second;
	}
	metrics.cacheMisses++;
	metrics.ruleExecutions++;  // Count both cache hits and misses
	std::cout << "[ValidationPipeline::evaluateRule] Cache miss, executing rule..." << std::endl;
	
	auto start = std::chrono::high_resolution_clock::now();
	
	// Try to execute the rule with timeout
	auto future = std::async(std::launch::async, [&]() {
		return metadata.rule->evaluate(score);
	});
	
	bool result = false;
	if (future.wait_for(ruleTimeout) == std::future_status::ready) {
		result = future.get();
		std::cout << "[ValidationPipeline::evaluateRule] Raw rule result: " << (result ? "true" : "false") << std::endl;
		if (!result) {
			std::string violation = metadata.rule->getName() + ": " + metadata.rule->getViolationDescription();
			violations.push_back(violation);
			metrics.violationsCount++;
			std::cout << "[ValidationPipeline::evaluateRule] Added violation: " << violation << std::endl;
		}
	} else {
		std::string violation = "Rule timed out: " + metadata.rule->getName();
		violations.push_back(violation);
		std::cout << "[ValidationPipeline::evaluateRule] Rule timed out" << std::endl;
		result = false;
	}
	
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	
	// Update metrics
	metadata.lastExecutionTime = duration.count();
	metadata.lastValidatedMeasure = measureIndex;
	metrics.totalExecutionTime += duration.count();
	metrics.maxExecutionTime = std::max(metrics.maxExecutionTime, duration.count());
	metrics.avgExecutionTime = metrics.totalExecutionTime / metrics.ruleExecutions;
	metrics.cacheHitRate = metrics.ruleExecutions > 0
		? static_cast<double>(metrics.cacheHits) / static_cast<double>(metrics.ruleExecutions)
		: 0.0;
	
	std::cout << "[ValidationPipeline::evaluateRule] Updated metrics:" << std::endl
			  << "  Total executions: " << metrics.ruleExecutions << std::endl
			  << "  Cache hits: " << metrics.cacheHits << std::endl
			  << "  Cache misses: " << metrics.cacheMisses << std::endl
			  << "  Cache hit rate: " << metrics.cacheHitRate << std::endl;
	
	// Cache the result
	std::cout << "[ValidationPipeline::evaluateRule] Caching result: " << (result ? "true" : "false") << std::endl;
	ruleCache[key] = result;
	
	// Return the result directly (true = rule satisfied, false = rule violated)
	std::cout << "[ValidationPipeline::evaluateRule] Returning: " << (result ? "satisfied" : "violated") << std::endl;
	return result;
}

void ValidationPipeline::sortRulesByDependencies() {
	evaluationOrder.clear();
	std::vector<bool> visited(rules.size(), false);
	std::vector<bool> inStack(rules.size(), false);
	
	std::function<void(size_t)> visit = [&](size_t index) {
		if (inStack[index]) {
			throw std::runtime_error("Circular dependency detected in rules");
		}
		if (visited[index]) return;
		
		inStack[index] = true;
		for (const auto& dep : rules[index].dependencies) {
			auto it = std::find_if(rules.begin(), rules.end(),
								 [&dep](const RuleMetadata& r) { return r.rule->getName() == dep; });
			if (it != rules.end()) {
				visit(std::distance(rules.begin(), it));
			}
		}
		inStack[index] = false;
		visited[index] = true;
		evaluationOrder.push_back(index);
	};
	
	for (size_t i = 0; i < rules.size(); ++i) {
		if (!visited[i]) {
			visit(i);
		}
	}
	
	std::reverse(evaluationOrder.begin(), evaluationOrder.end());
}

void ValidationPipeline::clearRuleCache() {
	ruleCache.clear();
	// Don't reset metrics here since we want to track them across multiple validations
	std::cout << "[ValidationPipeline::clearRuleCache] Cache cleared. Current metrics:" << std::endl
			  << "  Total executions: " << metrics.ruleExecutions << std::endl
			  << "  Cache hits: " << metrics.cacheHits << std::endl
			  << "  Cache misses: " << metrics.cacheMisses << std::endl
			  << "  Cache hit rate: " << metrics.cacheHitRate << std::endl;
}

std::vector<std::string> ValidationPipeline::getViolations() const {
	return violations;
}

void ValidationPipeline::clearViolations() {
	violations.clear();
	metrics.violationsCount = 0;
}

} // namespace music::rules