#include "domain/rules/ValidationPipeline.h"
#include "domain/state/SettingsState.h"
#include <algorithm>
#include <future>
#include <iostream>
#include <shared_mutex>
#include <QString>

// Using the global namespace qualifier for std
namespace MusicTrainer {
namespace music {
namespace rules {

::std::unique_ptr<ValidationPipeline> ValidationPipeline::create() {
    return ::std::unique_ptr<ValidationPipeline>(new ValidationPipeline());
}

void ValidationPipeline::addRule(::std::unique_ptr<Rule> rule, ::std::vector<::std::string> dependencies, int priority) {
    ::std::unique_lock<::std::shared_mutex> lock(rulesMutex);
    rules.emplace_back(::std::move(rule), ::std::move(dependencies), priority);
    compiled = false;
}

void ValidationPipeline::compileRules() {
    if (compiled) return;
    
    ::std::unique_lock<::std::shared_mutex> lock(rulesMutex);
    if (compiled) return; // Double-check after acquiring lock
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
#ifdef CLEAR_CACHE
    clearRuleCache();
#endif
    if (!compiled) {
        ::std::cout << "[ValidationPipeline] Rules not compiled, compiling now..." << ::std::endl;
        compileRules();
    }
    
    {
        ::std::lock_guard<::std::mutex> lock(violationsMutex);
        violations.clear();
    }
    
    bool allValid = true;
    
    // Use a shared lock for rules since we're only reading them
    ::std::shared_lock<::std::shared_mutex> rulesLock(rulesMutex);
    
    ::std::cout << "[ValidationPipeline] Starting validation with " << rules.size() << " rules" << ::std::endl;
    for (auto& metadata : rules) {
        ::std::cout << "[ValidationPipeline] Evaluating rule: " << metadata.rule->getName() << ::std::endl;
        bool ruleResult = evaluateRule(metadata, score, 0);
        ::std::cout << "[ValidationPipeline] Rule result: " << (ruleResult ? "valid" : "invalid") << ::std::endl;
        if (!ruleResult) {
            ::std::lock_guard<::std::mutex> lock(violationsMutex);
            ::std::cout << "[ValidationPipeline] Rule " << metadata.rule->getName() << " failed with violations:" << ::std::endl;
            for (const auto& violation : violations) {
                ::std::cout << "  - " << violation << ::std::endl;
            }
            allValid = false;
        }
    }
    
    // Update cache hit rate after all rules have been evaluated
    {
        ::std::lock_guard<::std::mutex> lock(metricsMutex);
        updateCacheHitRate();
    }
    
    ::std::cout << "[ValidationPipeline] Final result: " << (allValid ? "valid" : "invalid") << ::std::endl;
    {
        ::std::lock_guard<::std::mutex> lock(violationsMutex);
        ::std::cout << "[ValidationPipeline] Total violations: " << violations.size() << ::std::endl;
    }
    ::std::cout << "[ValidationPipeline] Returning: " << (allValid ? "valid" : "invalid") << ::std::endl;
    return allValid;  // true = all rules passed, false = at least one rule failed
}

bool ValidationPipeline::validateIncremental(const Score& score, size_t startMeasure) {
    if (!compiled) {
        ::std::cout << "[ValidationPipeline::validateIncremental] Rules not compiled, compiling now..." << ::std::endl;
        compileRules();
    }
    bool valid = true;
    
    // Use a shared lock for rules since we're only reading them
    ::std::shared_lock<::std::shared_mutex> rulesLock(rulesMutex);
    
    ::std::cout << "[ValidationPipeline::validateIncremental] Starting validation from measure " << startMeasure << ::std::endl;
    for (auto& metadata : rules) {
        // All rules should be evaluated in incremental validation
        ::std::cout << "[ValidationPipeline::validateIncremental] Evaluating rule: " << metadata.rule->getName() << ::std::endl;
        bool ruleResult = evaluateRule(metadata, score, startMeasure);
        ::std::cout << "[ValidationPipeline::validateIncremental] Rule result: " << (ruleResult ? "valid" : "invalid") << ::std::endl;
        if (!ruleResult) {
            ::std::lock_guard<::std::mutex> lock(violationsMutex);
            ::std::cout << "[ValidationPipeline::validateIncremental] Rule " << metadata.rule->getName() << " failed with violations:" << ::std::endl;
            for (const auto& violation : violations) {
                ::std::cout << "  - " << violation << ::std::endl;
            }
            valid = false;
        }
    }
    
    // Update cache hit rate after all rules have been evaluated
    {
        ::std::lock_guard<::std::mutex> lock(metricsMutex);
        updateCacheHitRate();
    }
    
    ::std::cout << "[ValidationPipeline::validateIncremental] Final result: " << (valid ? "valid" : "invalid") << ::std::endl;
    {
        ::std::lock_guard<::std::mutex> lock(violationsMutex);
        ::std::cout << "[ValidationPipeline::validateIncremental] Total violations: " << violations.size() << ::std::endl;
    }
    return valid;  // true = all rules passed, false = at least one rule failed
}

// Helper method to ensure cache hit rate is calculated consistently
void ValidationPipeline::updateCacheHitRate() {
    // This method is called with metricsMutex already held
    if (metrics.ruleExecutions > 0) {
        metrics.cacheHitRate = static_cast<double>(metrics.cacheHits) / static_cast<double>(metrics.ruleExecutions);
    } else {
        metrics.cacheHitRate = 0.0;
    }
}

// Fix the evaluateRule method to prevent memory corruption
bool ValidationPipeline::evaluateRule(RuleMetadata& metadata, const Score& score, size_t measureIndex) {
    ::std::cout << "\n[ValidationPipeline::evaluateRule] Starting evaluation of " << metadata.rule->getName() << ::std::endl;
    
    // Create cache key
    CacheKey key{
        metadata.rule->getName(),
        measureIndex,
        score.getHash()
    };
    ::std::cout << "[ValidationPipeline::evaluateRule] Cache key: name=" << key.ruleName
              << ", measure=" << key.measureIndex
              << ", hash=" << key.scoreHash << ::std::endl;
    
    // Check cache first with a lock
    {
        ::std::lock_guard<::std::mutex> cacheLock(cacheMutex);
        auto cacheIt = ruleCache.find(key);
        if (cacheIt != ruleCache.end()) {
            // Update metrics with proper lock
            {
                ::std::lock_guard<::std::mutex> metricsLock(metricsMutex);
                metrics.cacheHits++;
                metrics.ruleExecutions++;  // Count both cache hits and misses
                // Update the cache hit rate immediately
                updateCacheHitRate();
            }
            
            ::std::cout << "[ValidationPipeline::evaluateRule] Cache hit! Result: " << (cacheIt->second ? "valid" : "invalid") << ::std::endl;
            
            // For cache hits that represent violations, restore the violation from the violationCache
            if (!cacheIt->second) {
                auto violationIt = violationCache.find(key);
                if (violationIt != violationCache.end()) {
                    ::std::string violation = violationIt->second.message;
                    if (!violation.empty()) {
                        // Add violation with proper locks
                        {
                            ::std::lock_guard<::std::mutex> violationsLock(violationsMutex);
                            violations.push_back(violation);
                        }
                        
                        // Add feedback with proper lock
                        {
                            ::std::lock_guard<::std::mutex> feedbackLock(feedbackMutex);
                            feedbackItems.push_back({
                                violation,
                                violationIt->second.level,
                                measureIndex,
                                0
                            });
                        }
                        
                        ::std::cout << "[ValidationPipeline::evaluateRule] Restored cached violation: " << violation << ::std::endl;
                    } else {
                        ::std::cout << "[ValidationPipeline::evaluateRule] Warning: Empty violation message in cache" << ::std::endl;
                    }
                } else {
                    ::std::cout << "[ValidationPipeline::evaluateRule] Warning: No cached violation found for invalid rule result" << ::std::endl;
                }
            }
            return cacheIt->second;
        }
    }
    
    // Update metrics for cache miss with proper lock
    {
        ::std::lock_guard<::std::mutex> metricsLock(metricsMutex);
        metrics.cacheMisses++;
        metrics.ruleExecutions++;  // Count both cache hits and misses
        // Update the cache hit rate immediately
        updateCacheHitRate();
    }
    
    ::std::cout << "[ValidationPipeline::evaluateRule] Cache miss, executing rule..." << ::std::endl;
    auto start = ::std::chrono::high_resolution_clock::now();
    
    // Create a new Score from a snapshot to ensure proper lifetime management
    auto snapshot = score.createSnapshot();
    auto evalScore = ::std::make_unique<Score>(snapshot);
    
    // Try to execute the rule with timeout
    bool result = false;
    ::std::string violationMessage;
    
    try {
        // Create a single clone of the rule - the rule's clone method should handle its internal thread safety
        ::std::unique_ptr<Rule> clonedRule(metadata.rule->clone());
        ::std::cout << "[ValidationPipeline::evaluateRule] Cloned rule for evaluation, ptr=" << clonedRule.get() << ::std::endl;
        
        // Execute the rule directly on our Score copy
        result = clonedRule->evaluate(*evalScore);
        ::std::cout << "[ValidationPipeline::evaluateRule] Raw rule result: " << (result ? "true" : "false") << ::std::endl;
        
        // Get the violation description from the cloned rule if needed
        if (!result) {
            violationMessage = clonedRule->getViolationDescription();
            ::std::cout << "[ValidationPipeline::evaluateRule] Rule violation: " << violationMessage << ::std::endl;
            
            // Add to violations list with proper lock
            {
                ::std::lock_guard<::std::mutex> violationsLock(violationsMutex);
                violations.push_back(violationMessage);
            }
            
            // Update metrics with proper lock
            {
                ::std::lock_guard<::std::mutex> metricsLock(metricsMutex);
                metrics.violationsCount++;
            }
            
            // Determine feedback level
            FeedbackLevel level;
            if (violationMessage.find("large leap") != ::std::string::npos) {
                level = FeedbackLevel::WARNING;
            } else {
                level = FeedbackLevel::ERROR;
            }
            
            ValidationFeedback feedback{violationMessage, level, measureIndex, 0};
            
            // Add feedback with proper lock
            {
                ::std::lock_guard<::std::mutex> feedbackLock(feedbackMutex);
                feedbackItems.push_back(feedback);
            }
            
            // Store the violation in the cache for later restoration with proper lock
            {
                ::std::lock_guard<::std::mutex> cacheLock(cacheMutex);
                violationCache[key] = feedback;
            }
            
            ::std::cout << "[ValidationPipeline::evaluateRule] Added violation: " << violationMessage << ::std::endl;
        }
        auto end = ::std::chrono::high_resolution_clock::now();
        auto duration = ::std::chrono::duration_cast<::std::chrono::microseconds>(end - start);
        
        // Update metrics with proper lock
        {
            ::std::lock_guard<::std::mutex> metricsLock(metricsMutex);
            metadata.lastExecutionTime = duration.count();
            metadata.lastValidatedMeasure = measureIndex;
            metrics.totalExecutionTime += duration.count();
            metrics.maxExecutionTime = ::std::max(metrics.maxExecutionTime, duration.count());
            metrics.avgExecutionTime = metrics.totalExecutionTime / metrics.ruleExecutions;
            
            // Add rule timing to the metrics
            metrics.ruleTimings.emplace_back(metadata.rule->getName(), duration);
            
            ::std::cout << "[ValidationPipeline::evaluateRule] Updated metrics:" << ::std::endl
                    << "  Total executions: " << metrics.ruleExecutions << ::std::endl
                    << "  Cache hits: " << metrics.cacheHits << ::std::endl
                    << "  Cache misses: " << metrics.cacheMisses << ::std::endl
                    << "  Cache hit rate: " << metrics.cacheHitRate << ::std::endl;
        }
        
        // Cache the result with proper lock
        {
            ::std::lock_guard<::std::mutex> cacheLock(cacheMutex);
            ::std::cout << "[ValidationPipeline::evaluateRule] Caching result: " << (result ? "true" : "false") << ::std::endl;
            ruleCache[key] = result;
          
            // If the rule passed, remove any previous violation from the violationCache
            if (result) {
                violationCache.erase(key);
                ::std::cout << "[ValidationPipeline::evaluateRule] Removed violation from cache for passing rule" << ::std::endl;
            }
        }
        
        // Make sure clonedRule is properly destroyed by the unique_ptr
        clonedRule.reset();
        ::std::cout << "[ValidationPipeline::evaluateRule] Cleaned up cloned rule" << ::std::endl;
        
        // Clean up our evaluation score copy
        evalScore.reset();
        ::std::cout << "[ValidationPipeline::evaluateRule] Cleaned up score copy" << ::std::endl;
       
    } catch (const ::std::exception& e) {
        ::std::cerr << "[ValidationPipeline::evaluateRule] Exception during rule evaluation: " 
                << e.what() << ::std::endl;
        result = false;
    }
    
    // Return the result directly (true = rule satisfied, false = rule violated)
    ::std::cout << "[ValidationPipeline::evaluateRule] Returning: " << (result ? "satisfied" : "violated") << ::std::endl;
    return result;
}

void ValidationPipeline::sortRulesByDependencies() {
    // This method is called with rulesMutex already held
    evaluationOrder.clear();
    ::std::vector<bool> visited(rules.size(), false);
    ::std::vector<bool> inStack(rules.size(), false);
    
    ::std::function<void(size_t)> visit = [&](size_t index) {
        if (inStack[index]) {
            throw ::std::runtime_error("Circular dependency detected in rules");
        }
        if (visited[index]) return;
        
        inStack[index] = true;
        for (const auto& dep : rules[index].dependencies) {
            auto it = ::std::find_if(rules.begin(), rules.end(),
                                  [&dep](const RuleMetadata& r) { return r.rule->getName() == dep; });
            if (it != rules.end()) {
                visit(::std::distance(rules.begin(), it));
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
    
    ::std::reverse(evaluationOrder.begin(), evaluationOrder.end());
}

void ValidationPipeline::clearRuleCache() {
    ::std::lock_guard<::std::mutex> cacheLock(cacheMutex);
    ruleCache.clear();
    violationCache.clear();  // Also clear the violation cache
    
    // Don't reset metrics here since we want to track them across multiple validations
    {
        ::std::lock_guard<::std::mutex> metricsLock(metricsMutex);
        ::std::cout << "[ValidationPipeline::clearRuleCache] Cache cleared. Current metrics:" << ::std::endl
              << "  Total executions: " << metrics.ruleExecutions << ::std::endl
              << "  Cache hits: " << metrics.cacheHits << ::std::endl
              << "  Cache misses: " << metrics.cacheMisses << ::std::endl
              << "  Cache hit rate: " << metrics.cacheHitRate << ::std::endl;
    }
}

::std::vector<::std::string> ValidationPipeline::getViolations() const {
    ::std::lock_guard<::std::mutex> lock(violationsMutex);
    return violations;
}

void ValidationPipeline::clearViolations() {
    ::std::lock_guard<::std::mutex> violationsLock(violationsMutex);
    violations.clear();
    
    ::std::lock_guard<::std::mutex> metricsLock(metricsMutex);
    metrics.violationsCount = 0;
}

void ValidationPipeline::updateRules() {
    // Lock for thread safety while updating rules
    ::std::unique_lock<::std::shared_mutex> lock(rulesMutex);
    
    // Update each rule's enabled state based on settings
    for (auto& metadata : rules) {
        if (!metadata.rule) continue;
        
        ::std::string ruleName = metadata.rule->getName();
        bool enabled = ::MusicTrainer::state::SettingsState::instance().getRuleEnabled(ruleName);
        metadata.rule->setEnabled(enabled);
    }
    
    // Clear the cache since rule states have changed
    clearRuleCache();
    
    // Recompile rules to update evaluation order
    compileRules();
}

} // namespace rules
} // namespace music
} // namespace MusicTrainer