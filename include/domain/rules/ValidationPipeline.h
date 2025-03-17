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

namespace MusicTrainer::music::rules {

enum class FeedbackLevel {
    INFO,
    WARNING,
    ERROR
};

struct ValidationFeedback {
    ::std::string message;
    FeedbackLevel level;
    ::std::size_t measureIndex;
    ::std::size_t voiceIndex;
};

class ValidationPipeline {
public:
    struct RuleMetadata {
        ::std::unique_ptr<Rule> rule;
        ::std::vector<::std::string> dependencies;
        int priority;
        bool incremental;
        ::std::chrono::microseconds::rep lastExecutionTime{0};
        ::std::size_t lastValidatedMeasure{0};
        
        RuleMetadata(::std::unique_ptr<Rule> r, ::std::vector<::std::string> deps = {}, int p = 0)
            : rule(::std::move(r)), dependencies(::std::move(deps)), priority(p) {
            incremental = dynamic_cast<IncrementalRule*>(rule.get()) != nullptr;
        }
        
        RuleMetadata(const RuleMetadata& other)
            : dependencies(other.dependencies)
            , priority(other.priority)
            , incremental(other.incremental)
            , lastExecutionTime(other.lastExecutionTime)
            , lastValidatedMeasure(other.lastValidatedMeasure) {
            if (other.rule) {
                rule = ::std::unique_ptr<Rule>(other.rule->clone());
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
                    rule = ::std::unique_ptr<Rule>(other.rule->clone());
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
        ::std::chrono::microseconds::rep totalExecutionTime{0};
        ::std::chrono::microseconds::rep maxExecutionTime{0};
        ::std::chrono::microseconds::rep avgExecutionTime{0};
        ::std::size_t ruleExecutions{0};
        ::std::size_t cacheHits{0};
        ::std::size_t cacheMisses{0};
        double cacheHitRate{0.0};
        ::std::size_t violationsCount{0};
        ::std::vector<::std::pair<::std::string, ::std::chrono::microseconds>> ruleTimings;
    };
    
    static ::std::unique_ptr<ValidationPipeline> create();
    
    void addRule(::std::unique_ptr<Rule> rule, ::std::vector<::std::string> dependencies = {}, int priority = 0);
    void compileRules();
    void clearRuleCache();
    void updateRules();
    
    bool validate(const Score& score);
    bool validateIncremental(const Score& score, ::std::size_t startMeasure);
    
    ::std::vector<::std::string> getViolations() const;
    const ValidationMetrics& getMetrics() const { return metrics; }
    void clearViolations();
    
    const ::std::vector<ValidationFeedback>& getFeedback() const { return feedbackItems; }
    void clearFeedback() { feedbackItems.clear(); }

private:
    ::std::vector<ValidationFeedback> feedbackItems;
    ValidationPipeline();
    ::std::vector<RuleMetadata> rules;
    ::std::vector<::std::size_t> evaluationOrder;
    ::std::vector<::std::string> violations;
    ValidationMetrics metrics;
    ::std::atomic<bool> compiled{false};
    
    mutable ::std::mutex cacheMutex;
    mutable ::std::mutex violationsMutex;
    mutable ::std::mutex feedbackMutex;
    mutable ::std::mutex metricsMutex;
    mutable ::std::shared_mutex rulesMutex;
    
    void updateCacheHitRate();
    
    static constexpr ::std::chrono::milliseconds DEFAULT_RULE_TIMEOUT{5000};
    ::std::chrono::milliseconds ruleTimeout{DEFAULT_RULE_TIMEOUT};
    
    static constexpr ::std::chrono::milliseconds LOCK_TIMEOUT{1000};
    
    bool evaluateRule(RuleMetadata& metadata, const Score& score, ::std::size_t measureIndex);
    
    struct CacheKey {
        ::std::string ruleName;
        ::std::size_t measureIndex;
        ::std::size_t scoreHash;
        
        bool operator==(const CacheKey& other) const {
            return ruleName == other.ruleName && 
                   measureIndex == other.measureIndex && 
                   scoreHash == other.scoreHash;
        }
    };
    
    struct CacheKeyHash {
        ::std::size_t operator()(const CacheKey& key) const {
            return ::std::hash<::std::string>()(key.ruleName) ^ 
                   ::std::hash<::std::size_t>()(key.measureIndex) ^ 
                   ::std::hash<::std::size_t>()(key.scoreHash);
        }
    };
    
    ::std::unordered_map<CacheKey, bool, CacheKeyHash> ruleCache;
    ::std::unordered_map<CacheKey, ValidationFeedback, CacheKeyHash> violationCache;
    
    void sortRulesByDependencies();
};

} // namespace MusicTrainer::music::rules

#endif // MUSICTRAINERV3_VALIDATIONPIPELINE_H