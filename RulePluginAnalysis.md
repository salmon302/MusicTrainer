# Rule Plugin Analysis

## 1. Rule Plugin Architecture

### 1.1 Core Rule Interface
```cpp
class RuleValidation {
	struct ValidationContext {
		const Score* score;
		const Voice* activeVoice;
		ValidationPriority priority;
		std::atomic<bool> shouldContinue;
	};
	
	class RuleExecution {
		ValidationResult validate(const ValidationContext& ctx);
		bool canExecuteInParallel() const;
		ValidationPriority getPriority() const;
	};
};
```

### 1.2 Plugin Integration
```cpp
class RulePluginFramework {
	// Plugin configuration
	struct PluginConfig {
		std::string id;
		Version version;
		nlohmann::json parameters;
		ValidationMode mode;
	};
	
	// Rule factory
	class RuleFactory {
		std::shared_ptr<Rule> createRule(const PluginConfig& config);
		bool validateConfig(const nlohmann::json& config);
	};
};
```

## 2. Validation Strategy

### 2.1 Performance Optimization
- Rule caching
- Incremental validation
- Parallel execution
- Early termination

### 2.2 Rule Dependencies
```cpp
class RuleDependencyManager {
	// Dependency graph
	struct DependencyNode {
		RuleId ruleId;
		std::vector<RuleId> dependencies;
		ValidationOrder order;
	};
	
	// Execution planning
	class ExecutionPlanner {
		std::vector<RuleId> getValidationOrder();
		bool detectCycles();
		void optimizeExecution();
	};
};
```

## 3. State Integration

### 3.1 Rule State Management
```cpp
class RuleStateManager {
	// Rule state tracking
	struct RuleState {
		ValidationCache cache;
		std::atomic<bool> isValid;
		TimeStamp lastValidation;
	};
	
	// State synchronization
	class StateSync {
		void updateRuleState(const ValidationResult& result);
		void invalidateCache(const Score& score);
		void propagateStateChanges();
	};
};
```

### 3.2 Event Handling
- Rule validation events
- State change notifications
- Error propagation
- Cache invalidation

## 4. Error Handling

### 4.1 Validation Errors
```cpp
class ValidationErrorHandler {
	// Error classification
	struct ValidationError {
		ErrorType type;
		Severity severity;
		std::string message;
		Recovery recovery;
	};
	
	// Error recovery
	class ErrorRecovery {
		bool attemptRecovery(const ValidationError& error);
		void rollbackChanges();
		void notifyError();
	};
};
```

### 4.2 Plugin Errors
- Load failures
- Configuration errors
- Runtime exceptions
- Resource exhaustion

## 5. Testing Strategy

### 5.1 Rule Tests
```cpp
class RuleTestFramework {
	// Test cases
	struct RuleTest {
		Score testScore;
		ExpectedResult expected;
		ValidationContext context;
	};
	
	// Test execution
	class TestRunner {
		void runValidationTests();
		void checkPerformance();
		void verifyCorrectness();
	};
};
```

### 5.2 Integration Tests
- Cross-plugin validation
- State consistency
- Error handling
- Performance impact

## 6. Performance Considerations

### 6.1 Optimization Strategies
- Rule execution ordering
- Cache utilization
- Memory management
- Thread utilization

### 6.2 Resource Management
```cpp
class RuleResourceManager {
	// Resource tracking
	struct ResourceUsage {
		size_t memoryUsage;
		CPU_Time cpuTime;
		size_t cacheSize;
	};
	
	// Resource optimization
	class ResourceOptimizer {
		void optimizeMemory();
		void balanceThreads();
		void manageCacheSize();
	};
};
```

## 7. Implementation Guidelines

### 7.1 Best Practices
- Thread safety requirements
- Error handling patterns
- State management patterns
- Performance optimization

### 7.2 Plugin Development
```cpp
class PluginDevelopmentKit {
	// Plugin template
	struct PluginTemplate {
		ValidationInterface interface;
		ConfigurationSchema schema;
		TestFramework tests;
	};
	
	// Development tools
	class PluginTools {
		void validateImplementation();
		void checkPerformance();
		void generateDocs();
	};
};

## 8. Species Counterpoint Presets

### 8.1 Common Rule Components
```cpp
struct IntervalRule {
    std::set<Interval> allowedMelodicIntervals;
    std::set<Interval> allowedHarmonicIntervals;
    bool allowParallelPerfectConsonances;
    Direction approachToPerfectConsonance;
};

struct RhythmicRule {
    NoteDuration cantusNoteLength;
    NoteDuration counterPointNoteLength;
    int notesAgainstOne;
    bool allowSyncopation;
};

struct MotionRule {
    bool requireContrary;
    bool allowParallel;
    bool allowOblique;
    double contraryMotionPercentage;
};
```

### 8.2 Species-Specific Presets

#### 8.2.1 First Species
```cpp
struct FirstSpeciesPreset {
    IntervalRule intervals {
        .allowedMelodicIntervals = {Unison, M2, m2, M3, m3, P4, P5, P8},
        .allowedHarmonicIntervals = {P1, P5, P8, M3, m3, M6, m6},
        .allowParallelPerfectConsonances = false,
        .approachToPerfectConsonance = Direction::Contrary
    };
    RhythmicRule rhythm {
        .cantusNoteLength = Whole,
        .counterPointNoteLength = Whole,
        .notesAgainstOne = 1,
        .allowSyncopation = false
    };
    MotionRule motion {
        .requireContrary = true,
        .allowParallel = true,
        .allowOblique = true,
        .contraryMotionPercentage = 0.75
    };
};
```

#### 8.2.2 Second Species
```cpp
struct SecondSpeciesPreset : FirstSpeciesPreset {
    RhythmicRule rhythm {
        .cantusNoteLength = Whole,
        .counterPointNoteLength = Half,
        .notesAgainstOne = 2,
        .allowSyncopation = false
    };
    PassingToneRule passingTones {
        .allowDissonantPassing = true,
        .onWeakBeatOnly = true
    };
};
```

#### 8.2.3 Third Species
```cpp
struct ThirdSpeciesPreset : FirstSpeciesPreset {
    RhythmicRule rhythm {
        .cantusNoteLength = Whole,
        .counterPointNoteLength = Quarter,
        .notesAgainstOne = 4,
        .allowSyncopation = false
    };
    struct Ornamentation {
        bool allowNeighborTones = true;
        bool allowDoublePassing = true;
        bool allowCambiata = true;
    };
};
```

#### 8.2.4 Fourth Species
```cpp
struct FourthSpeciesPreset : FirstSpeciesPreset {
    RhythmicRule rhythm {
        .cantusNoteLength = Whole,
        .counterPointNoteLength = Whole,
        .notesAgainstOne = 1,
        .allowSyncopation = true
    };
    struct Suspension {
        bool required = true;
        std::set<Interval> allowedDissonances = {m7, M7, m2, M2};
        Resolution resolution = StepDown;
    };
};
```

#### 8.2.5 Fifth Species (Florid)
```cpp
struct FloridCounterpointPreset {
    std::vector<std::shared_ptr<CounterpointPreset>> availableSpecies;
    struct Constraints {
        double maxConsecutiveSameSpecies = 2;
        bool requireAllSpecies = true;
        double minSpeciesVariety = 0.6;
    };
};
```

#### 8.2.6 Free Counterpoint
```cpp
struct FreeCounterpointPreset {
    IntervalRule intervals {
        .allowedMelodicIntervals = AllIntervals,
        .allowedHarmonicIntervals = ConsonantIntervals,
        .allowParallelPerfectConsonances = true,
        .approachToPerfectConsonance = Direction::Any
    };
    bool enforceVoiceLeading = false;
    bool allowChromaticism = true;
    bool allowModulation = true;
};
```

#### 8.2.7 Linear Counterpoint
```cpp
struct LinearCounterpointPreset {
    bool prioritizeHorizontal = true;
    bool enforceHarmonicRules = false;
    struct VoiceIndependence {
        double minIndependenceScore = 0.8;
        bool allowVoiceCrossing = true;
        bool allowUnisons = false;
    };
};
```

#### 8.2.8 Dissonant Counterpoint
```cpp
struct DissonantCounterpointPreset {
    IntervalRule intervals {
        .allowedMelodicIntervals = AllIntervals,
        .allowedHarmonicIntervals = DissonantIntervals,
        .allowParallelPerfectConsonances = false,
        .approachToPerfectConsonance = Direction::Skip
    };
    struct DissonanceRules {
        double minDissonancePercentage = 0.7;
        bool resolveToConsonance = false;
        bool requireStepwiseResolution = false;
    };
};
```

### 8.3 Preset Configuration System
```cpp
class CounterpointPresetManager {
    struct PresetConfig {
        std::string presetName;
        nlohmann::json ruleParameters;
        ValidationMode mode;
    };

    // Customization interface
    void customizeIntervals(const std::set<Interval>& allowed);
    void setMotionRules(const MotionRule& rules);
    void setRhythmicRules(const RhythmicRule& rules);
    
// Validation and application
bool validatePresetCompatibility(const PresetConfig& config);
std::shared_ptr<RuleSet> generateRuleSet(const PresetConfig& config);

// Melodic validation
struct MelodicValidation {
	bool validateStepwiseMotion;
	bool validateLeapCompensation;
	bool validateClimaxPoint;
	std::set<Interval> forbiddenMelodicPatterns;
};

// Harmonic validation
struct HarmonicValidation {
	bool validateParallelMotion;
	bool validateVoiceCrossing;
	bool validateVoiceOverlapping;
	std::set<Interval> forbiddenHarmonicPatterns;
};

// Cadence validation
struct CadenceValidation {
	bool requireLeadingTone;
	bool requireStepwiseApproach;
	std::set<Interval> allowedFinalIntervals;
};

// Interval customization
struct IntervalCustomization {
	void addAllowedMelodicInterval(Interval interval);
	void removeAllowedMelodicInterval(Interval interval);
	void setHarmonicIntervalTolerance(Interval interval, double tolerance);
	void defineCustomIntervalProgression(std::vector<Interval> progression);
};

// Motion customization
struct MotionCustomization {
	void setMotionPreference(MotionType type, double weight);
	void addForbiddenMotionPattern(std::vector<MotionType> pattern);
	void setContraryMotionThreshold(double percentage);
};

// Rule priority
struct RulePriority {
	void setPriorityLevel(RuleType type, int priority);
	void setRuleWeight(RuleType type, double weight);
	void defineRuleDependency(RuleType dependent, RuleType dependency);
};

// Exercise integration
struct ExerciseIntegration {
	void applyPresetToExercise(const CounterpointPreset& preset);
	void validateExerciseAgainstPreset(const Exercise& exercise);
	std::vector<RuleViolation> getPresetViolations();
};

// Plugin system integration
struct PluginIntegration {
	void registerCustomPreset(const CounterpointPreset& preset);
	void extendPresetRules(const PresetExtension& extension);
	bool validatePresetCompatibility(const Plugin& plugin);
};

// State management
struct StateIntegration {
	void updatePresetState(const PresetState& state);
	void synchronizePresetRules(const RuleSet& rules);
	void handlePresetStateTransition(const StateTransition& transition);
};

// Progressive validation
struct ProgressiveValidation {
	void validateByMeasure(const Score& score);
	void validateByPhrase(const Score& score);
	void validateEntireComposition(const Score& score);
};

// Real-time validation
struct RealtimeValidation {
	void validateNoteInput(const Note& note);
	void validateVoiceLeading(const Voice& voice);
	void predictPotentialViolations(const Score& score);
};

// Performance optimization
struct ValidationOptimization {
	void cacheValidationResults();
	void prioritizeRuleExecution();
	void parallelizeValidation();
	void incrementalValidation(const ScoreRegion& region);
};