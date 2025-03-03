#ifndef MUSICTRAINERV3_VALIDATIONPIPELINE_H
#define MUSICTRAINERV3_VALIDATIONPIPELINE_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include "Rule.h"
#include "IncrementalRule.h"
#include "../music/Score.h"

namespace MusicTrainer {
namespace music {
namespace rules {

enum class FeedbackLevel {
    INFO,
    WARNING,
    ERROR
};

struct ValidationFeedback {
    std::string message;
    FeedbackLevel level;
    size_t measureIndex;
    size_t voiceIndex;
};

class ValidationPipeline {
public:
    struct RuleMetadata {
        std::unique_ptr<Rule> rule;
        std::vector<std::string> dependencies;
        int priority;
        bool incremental;
        std::chrono::microseconds::rep lastExecutionTime{0};
        size_t lastValidatedMeasure{0};
        
        RuleMetadata(std::unique_ptr<Rule> r, std::vector<std::string> deps = {}, int p = 0)
            : rule(std::move(r)), dependencies(std::move(deps)), priority(p) {
            incremental = dynamic_cast<IncrementalRule*>(rule.get()) != nullptr;
        }
        
        RuleMetadata(const RuleMetadata& other)
            : dependencies(other.dependencies)
            , priority(other.priority)
            , incremental(other.incremental)
            , lastExecutionTime(other.lastExecutionTime)
            , lastValidatedMeasure(other.lastValidatedMeasure) {
            if (other.rule) {
                rule = std::unique_ptr<Rule>(other.rule->clone());
            }
        }
        
        RuleMetadata& operator=(const RuleMetadata& other) {
            if (this != &other) {
                dependencies = other.dependencies;
                priority = other.priority;
                incremental = other.incremental;
                lastExecutionTime = other.lastExecutionTime;
                lastValidatedMeasure = other.lastValidatedMeasure;
                if (other.rule) {
                    rule = std::unique_ptr<Rule>(other.rule->clone());
                } else {
                    rule.reset();
                }
            }
            return *this;
        }
        
        RuleMetadata(RuleMetadata&& other) noexcept = default;
        RuleMetadata& operator=(RuleMetadata&& other) noexcept = default;
    };
    
    struct ValidationMetrics {
        std::chrono::microseconds::rep totalExecutionTime{0};
        std::chrono::microseconds::rep maxExecutionTime{0};
        std::chrono::microseconds::rep avgExecutionTime{0};
        size_t ruleExecutions{0};
        size_t cacheHits{0};
        size_t cacheMisses{0};
        double cacheHitRate{0.0};
        size_t violationsCount{0};
        std::vector<std::pair<std::string, std::chrono::microseconds>> ruleTimings;
    };
    
    static std::unique_ptr<ValidationPipeline> create();
    
    // Enhanced rule management
    void addRule(std::unique_ptr<Rule> rule, std::vector<std::string> dependencies = {}, int priority = 0);
    void compileRules();
    void clearRuleCache();
    
    // Validation
    // Lock ordering for validation methods:
    // 1. Voice lock (Level 0)
    // 2. Score lock (Level 1) 
    // 3. Validation lock (Level 4)
    // 4. Metrics lock (Level 5)
    
    // Validates the entire score using compiled rules
    // Returns: true if all rules pass, false otherwise
    bool validate(const Score& score);
    
    // Validates score incrementally from given measure
    // Returns: true if all affected rules pass, false otherwise 
    bool validateIncremental(const Score& score, size_t startMeasure);
    
    // Results and metrics
    std::vector<std::string> getViolations() const;
    const ValidationMetrics& getMetrics() const { return metrics; }
    void clearViolations();
    
    const std::vector<ValidationFeedback>& getFeedback() const { return feedbackItems; }
    void clearFeedback() { feedbackItems.clear(); }

private:
    std::vector<ValidationFeedback> feedbackItems;
    ValidationPipeline();
    std::vector<RuleMetadata> rules;
    std::vector<size_t> evaluationOrder;
    std::vector<std::string> violations;
    ValidationMetrics metrics;
    std::atomic<bool> compiled{false};
    
    // Mutex for thread safety
    mutable std::mutex cacheMutex;
    mutable std::mutex violationsMutex;
    mutable std::mutex feedbackMutex;
    mutable std::mutex metricsMutex;
    mutable std::shared_mutex rulesMutex;
    
    // Helper method to update cache hit rate calculation
    void updateCacheHitRate();
    
    // Timeout settings
    static constexpr std::chrono::milliseconds DEFAULT_RULE_TIMEOUT{5000}; // 5 second timeout for rules
    std::chrono::milliseconds ruleTimeout{DEFAULT_RULE_TIMEOUT};
    
    // Lock timeout settings
    static constexpr std::chrono::milliseconds LOCK_TIMEOUT{1000}; // 1 second timeout for locks
    
    // Evaluates a single rule with proper lock ordering and timeout handling
    // Lock ordering:
    // 1. Check cache under METRICS lock
    // 2. Execute rule (no locks held)
    // 3. Update metrics/cache under METRICS lock
    // Returns: true if rule passes, false if rule fails or times out
    bool evaluateRule(RuleMetadata& metadata, const Score& score, size_t measureIndex);
    
    // Cache for rule results
    struct CacheKey {
        std::string ruleName;
        size_t measureIndex;
        size_t scoreHash;
        
        bool operator==(const CacheKey& other) const {
            return ruleName == other.ruleName && 
                   measureIndex == other.measureIndex && 
                   scoreHash == other.scoreHash;
        }
    };
    
    struct CacheKeyHash {
        size_t operator()(const CacheKey& key) const {
            return std::hash<std::string>()(key.ruleName) ^ 
                   std::hash<size_t>()(key.measureIndex) ^ 
                   std::hash<size_t>()(key.scoreHash);
        }
    };
    
    std::unordered_map<CacheKey, bool, CacheKeyHash> ruleCache;
    
    // Store feedback information for each violation in the cache
    std::unordered_map<CacheKey, ValidationFeedback, CacheKeyHash> violationCache;
    
    void sortRulesByDependencies(std::vector<RuleMetadata>& rulesCopy, std::vector<size_t>& order);
    void sortRulesByDependencies(); // Keep original method for backward compatibility
};

} // namespace rules
} // namespace music
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_VALIDATIONPIPELINE_H