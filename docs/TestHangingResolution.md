# Test Hanging Resolution Guide

## Issue: ValidationPipelineTest Hanging

### Symptoms
- Test hangs during `ValidationPipelineTest.DetectsParallelFifths`
- Last output shows: `[Pipeline::evaluateRule] Evaluating rule: Parallel Fifths Rule`
- No error message or timeout, just infinite hanging
- Test output trace:
```
[Score::addVoice] Adding voice to score
[Score::addVoice] Voice added
[Score::addVoice] Adding voice to score
[Score::addVoice] Voice added
[Pipeline::compileRules] Starting rule compilation...
[Pipeline::sortRulesByDependencies] Starting topological sort...
[Pipeline::sortRulesByDependencies] Visiting rule: Parallel Fifths Rule
[Pipeline::sortRulesByDependencies] Sort complete. Order size: 1
[Pipeline::compileRules] Rule compilation complete.
[Pipeline] Starting validation...
[Pipeline] Evaluating rule: Parallel Fifths Rule
[Pipeline::evaluateRule] Evaluating rule: Parallel Fifths Rule
[test hangs here]
```

### Root Cause Analysis
1. Thread Safety Issues:
   - Recursive mutex locking in rule evaluation
   - Potential deadlock between rule execution and cache access
   - Possible race condition in metrics update

2. Critical Code Paths:
   - ValidationPipeline::evaluateRule()
   - ValidationPipeline::validate()
   - Rule execution and cache management

### Resolution Steps

1. Mutex Strategy Revision:
```cpp
// Update in ValidationPipeline.h
private:
    mutable std::mutex mutex_;
    mutable std::mutex metrics_mutex_;  // Separate mutex for metrics
```

2. Rule Execution Isolation:
```cpp
// Update in ValidationPipeline.cpp
bool ValidationPipeline::evaluateRule(RuleMetadata& metadata, const Score& score, size_t measureIndex) {
    CacheKey key;
    bool useCache = false;
    bool cacheHit = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        key = CacheKey{metadata.rule->getName(), measureIndex, score.getHash()};
        auto cacheIt = ruleCache.find(key);
        if (cacheIt != ruleCache.end()) {
            useCache = true;
            cacheHit = true;
        }
    }

    if (useCache) {
        std::lock_guard<std::mutex> metrics_lock(metrics_mutex_);
        updateMetrics(metrics, true);
        return cacheHit;
    }

    // Execute rule without any locks
    bool result = executeRule(metadata, score, measureIndex);

    // Update cache and metrics separately
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ruleCache[key] = result;
    }
    {
        std::lock_guard<std::mutex> metrics_lock(metrics_mutex_);
        updateMetrics(metrics, false);
    }

    return result;
}
```

3. Safe Rule Execution:
```cpp
namespace {
    bool executeRule(ValidationPipeline::RuleMetadata& metadata, const Score& score, size_t measureIndex) {
        try {
            auto start = std::chrono::high_resolution_clock::now();
            bool result;
            
            if (metadata.incremental) {
                auto* incrementalRule = static_cast<IncrementalRule*>(metadata.rule.get());
                result = incrementalRule->evaluateIncremental(score, measureIndex, measureIndex + 1);
            } else {
                result = metadata.rule->evaluate(score);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            metadata.lastExecutionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            return result;
        } catch (const std::exception& e) {
            std::cerr << "[executeRule] Error: " << e.what() << std::endl;
            throw;
        }
    }
}
```

### Testing the Fix
1. Run with debug logging:
```bash
set GTEST_BREAK_ON_FAILURE=1
set GTEST_VERBOSE=1
ctest -R ValidationPipelineTest.DetectsParallelFifths -V
```

2. Verify thread safety:
- No recursive mutex locking
- Separate metrics and cache updates
- Isolated rule execution

### Prevention Measures
1. Thread Safety:
   - Use separate mutexes for different concerns
   - Minimize lock scope
   - Avoid nested locks
   - Implement RAII lock guards

2. Rule Execution:
   - Execute rules without holding locks
   - Cache results atomically
   - Update metrics safely
   - Handle exceptions properly

3. Testing:
   - Add thread safety tests
   - Implement stress testing
   - Monitor lock contention
   - Add timeout mechanism

### Related Components
- ValidationPipeline
- Rule execution system
- Metrics tracking
- Cache management
- Thread synchronization

This resolution addresses the core issues causing test hanging by improving thread safety and lock management while maintaining the validation pipeline's functionality.