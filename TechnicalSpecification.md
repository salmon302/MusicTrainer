# Technical Specification

## High-Level Architectural Overview

MusicTrainer V4 is a layered desktop application (C++/Qt6) designed for advanced counterpoint education. The architecture emphasizes modularity, testability, and performance, adhering to the requirements specified in `SRS_V4.md`. The core components are:

1.  **Domain Model:** Immutable representation of musical concepts (notes, scores, intervals, voices, time/key signatures). Independent of UI and external systems. (Aligns with `productContext.md`, `SRS_V4.md` 2.4)
2.  **Rule Engine:** Validates scores against configurable rule sets (melodic, harmonic, rhythmic) via a plugin architecture. Provides detailed feedback. (SRS V4: 3.1.4, FR-xx)
3.  **Preset System:** Manages loading, saving, and applying rule configurations (Presets). (SRS V4: 3.1.6, FP-xx)
4.  **Solution/Hint Generator:** Provides context-aware hints and generates valid solutions based on active rules using search algorithms (e.g., A*). (SRS V4: 3.1.8, FS-xx)
5.  **Score Viewport:** Interactive piano-roll style display with efficient rendering, navigation (zoom/pan), note manipulation, and feedback visualization. (SRS V4: 3.1.1, FV-xx)
6.  **MIDI System:** Handles low-latency MIDI I/O, device management, and event processing. (SRS V4: 3.1.3, FM-xx, NF-PERF-01)
7.  **Exercise Management:** Loads, manages, and potentially generates exercises and learning paths. (SRS V4: 3.1.5, FE-xx)
8.  **Feedback/Analysis System:** Consolidates validation results and analytical data (voice characteristics, intervals) for presentation. (SRS V4: 3.1.7, FF-xx)
9.  **Persistence Service:** Handles serialization/deserialization of user settings, presets, and exercise/score files. (SRS V4: 4, FD-xx)
10. **Command/Undo System:** Manages user actions for undo/redo functionality. (SRS V4: FN-10)
11. **Application Core/State Management:** Orchestrates component interactions, manages application state, and handles events. (SRS V4: 2.4, `StateManagement.md`)
12. **UI Layer (Qt6):** Provides the graphical user interface, including main window, panels, dialogs, and controls. (SRS V4: 3.1.9, 3.3.1, FU-xx, NF-UI-xx)
13. **Dev/Testing Interface:** Provides CLI access and structured logging for automated testing and agent interaction. (SRS V4: 3.1.10, FDV-xx)

These components interact primarily through signals/slots (Qt) or defined interfaces, following an event-driven approach aligned with the MVVM pattern for the UI.

## 1. Persistence Service

This service is responsible for saving and loading application data as required by `SRS_V4.md` Section 4 (FD-xx).

*   **User Settings (FD-01):** Stored in a platform-appropriate configuration file (e.g., INI, JSON) in the user's application data directory. Includes MIDI device selections, UI preferences (e.g., voice colors), last used directories, etc. Uses Qt's `QSettings` or a similar mechanism.
*   **Presets (FD-02, FP-04):** Saved as individual JSON files in a dedicated user presets directory. The JSON schema defines rule enablement, parameter values, and metadata (name, description). See Section 2.1 for Preset System details.
*   **Exercise/Score Files (FD-04, FD-05, FD-06):** Saved in a custom project file format (e.g., `.mtx`). This format needs to store:
    *   Score data: Notes (pitch, position, duration, velocity), voices, time signature, key signature.
    *   Associated Preset: Reference to the preset file (or embedded preset data) used for the exercise.
    *   Exercise metadata: Title, description, goals, Cantus Firmus (if applicable).
    *   (Future: User progress state).
    *   Serialization format TBD (JSON, binary, or XML). Must be robust against corruption (NF-REL-02).
*   **Built-in Exercises (FD-03):** Packaged within the application resources (e.g., Qt Resource System) using the same `.mtx` format as user-saved files.

## 2. Core Systems Implementation

### 2.1 Rule Engine (SRS V4: 3.1.4)

```cpp
// Represents a single validation failure
struct Violation {
    RuleId ruleId;
    std::string description;
    Severity severity; // e.g., Error, Warning
    std::vector<Note*> involvedNotes;
    TimeRange location;
};

// Result of a validation pass
struct ValidationResult {
    std::vector<Violation> violations;
    bool isValid;
};

// Interface for individual rules (Plugin)
class IRule {
public:
    virtual ~IRule() = default;
    virtual RuleId getId() const = 0;
    virtual std::string getName() const = 0;
    virtual RuleType getType() const = 0; // Melodic, Harmonic, Rhythmic
    virtual void configure(const RuleParameters& params) = 0; // FR-09
    virtual void validate(const Score& score, ValidationResult& result) = 0;
    virtual std::vector<RuleId> getDependencies() const = 0; // For conflict detection
};

// Core rule engine interface
class RuleEngine {
public:
    virtual ValidationResult validate(const Score& score, const TimeRange& dirtyRegion) = 0; // Incremental validation
    virtual void loadRuleSet(const Preset& preset) = 0; // Applies rules and parameters from preset
    virtual void registerRulePlugin(std::shared_ptr<IRule> rulePlugin) = 0; // FR-10
    virtual std::vector<std::string> checkRuleConflicts(const Preset& preset) = 0; // FR-11, FP-09
    virtual const RuleParameters& getRuleParameters(RuleId id) const = 0;
};
```

*   **Functionality:** Validates scores against active rules defined by the loaded Preset (FR-01). Supports Melodic, Harmonic, and Rhythmic rule types (FR-05, FR-06, FR-07).
*   **Architecture:** Employs a plugin architecture (FR-10) where individual rules implement the `IRule` interface. Rules are registered dynamically.
*   **Configuration:** Rules accept parameters (FR-09) defined in the Preset file.
*   **Performance:** Uses incremental validation triggered by score changes (NF-PERF-04, Section 3.1). Caching of intermediate results (e.g., interval calculations) is employed. Validation aims for < 50ms response time.
*   **Conflict Detection:** Analyzes rule dependencies (declared via `getDependencies`) to detect potential conflicts within a Preset before application (FR-11, FP-09).
*   **Feedback:** Generates detailed `ValidationResult` containing specific `Violation` objects, including location and involved notes, for use by the Feedback/Analysis System (FR-03, FR-04).

### 2.2 Solution/Hint Generator (SRS V4: 3.1.8)

```cpp
// Represents a hint or suggestion
struct Hint {
    HintLevel level; // e.g., General, SpecificInterval, ExactNote
    std::string description;
    std::vector<Note> suggestedNotes; // Optional, for specific hints
    TimeRange location;
};

// Interface for solution/hint generation
class SolutionGenerator {
public:
    // Generates hints based on current score state and violations
    virtual std::vector<Hint> generateHints(const Score& currentScore, const ValidationResult& currentViolations, int requestCount) = 0; // FS-01, FS-02, FS-03 (requestCount for progressive hints)

    // Generates a complete, valid solution for the exercise context
    virtual std::optional<Score> generateSolution(const ExerciseContext& context) = 0; // FS-05, FS-06

    // Checks if a user's score is a valid solution
    virtual bool checkSolution(const Score& userScore, const ExerciseContext& context) = 0;
};
```

*   **Functionality:** Provides context-aware hints (FS-01, FS-02) and generates complete, valid solutions (FS-05, FS-06) for exercises based on the active rule set.
*   **Algorithms:** Uses constraint satisfaction techniques and heuristic search algorithms (e.g., A*, backtracking) informed by the Rule Engine's validation logic (FS-04). Section 5.1 details the A* approach.
*   **Hints:** Offers progressive hints (FS-03) with increasing specificity based on repeated requests. Hints focus on resolving existing violations or suggesting valid next steps.
*   **Performance:** Solution generation aims for reasonable performance (FS-08, NF-PERF-07), potentially using background threads for complex cases. Hint generation must be faster.
*   **Integration:** Relies heavily on the Rule Engine for validating potential notes/solutions (FR-12).

### 2.3 Score Viewport (SRS V4: 3.1.1)

```cpp
// Interface for viewport interaction and rendering logic
class ScoreViewport {
public:
    // Rendering
    virtual void render(QPainter& painter, const ViewportState& state) = 0; // NF-PERF-02 (60 FPS target)
    virtual void updateVisibleNotes(const Score& score, const ViewportState& state) = 0;

    // Navigation & State
    virtual void setViewportState(const ViewportState& state) = 0; // Zoom, Pan FV-08, FV-09, FV-10
    virtual ViewportState getViewportState() const = 0;
    virtual void centerOn(const TimeRange& range, const PitchRange& pitchRange) = 0; // FV-19
    virtual void setFixedPitchRange(const std::optional<PitchRange>& range) = 0; // FV-22, FV-23
    virtual void setAxisScaling(float timeScale, float pitchScale) = 0; // FV-20, FV-21

    // Interaction
    virtual std::optional<MusicalCoordinate> screenToMusical(const QPoint& screenPos) = 0;
    virtual QPoint musicalToScreen(const MusicalCoordinate& musicalPos) = 0;
    virtual std::vector<Note*> getNotesAt(const QPoint& screenPos) = 0;
    virtual void showHoverFeedback(const QPoint& screenPos) = 0; // FV-14
    virtual void handleNoteSelection(const QRect& selectionRect, bool shiftModifier) = 0; // FV-15, FV-16
    virtual void showViolationFeedback(const std::vector<Violation>& violations) = 0; // FR-03, NF-UI-04

    // Grid Management (Integration with GridExpansionManager)
    virtual void notifyBoundsChange(const Rectangle& newLogicalBounds) = 0; // FV-12, FV-13
};

// (GridExpansionManager and ViewportStateManager remain similar to V3 spec,
//  but integrate with ScoreViewport interface and performance optimizations in Section 3.4)
```

*   **Functionality:** Displays the score interactively (FV-01 to FV-07), handles user navigation (zoom/pan FV-08, FV-09, FV-10, FV-11, FV-20, FV-22), manages note selection (FV-15, FV-16, FV-17), and provides visual feedback (hover FV-14, violations FR-03).
*   **Performance:** Optimized for smooth rendering (60 FPS target NF-PERF-02) and navigation even with large scores (FV-18). Uses techniques detailed in Section 3.4 (caching, lazy expansion, etc.). Dynamic grid expansion (FV-12, FV-13) is handled efficiently (NF-PERF-08).
*   **Interaction:** Translates between screen coordinates and musical coordinates. Provides mechanisms for note input/editing tools to interact with the score display (linking to FN-xx requirements).
*   **Feedback:** Renders visual cues for rule violations provided by the Rule Engine/Feedback System.

### 2.4 MIDI System (SRS V4: 3.1.3)

```cpp
// Represents a MIDI message
struct MidiEvent {
    enum class Type { NoteOn, NoteOff, ControlChange, ... };
    Type type;
    int channel;
    int data1; // e.g., Note Number, CC Number
    int data2; // e.g., Velocity, CC Value
    uint64_t timestamp; // High-resolution timestamp
};

// Interface for MIDI I/O
class MidiSystem {
public:
    virtual std::vector<std::string> listInputDevices() = 0; // FM-01
    virtual std::vector<std::string> listOutputDevices() = 0; // FM-02
    virtual bool openInputDevice(const std::string& deviceName) = 0;
    virtual bool openOutputDevice(const std::string& deviceName) = 0;
    virtual void closeInputDevice() = 0;
    virtual void closeOutputDevice() = 0;

    // Input Handling (Connects to Note Input FN-02, FN-03, FN-05)
    virtual void setInputCallback(std::function<void(const MidiEvent&)> callback) = 0;
    virtual void setChannelMapping(int inputChannel, int voiceIndex) = 0; // FM-11

    // Output Handling (Connects to Playback FU-04, Metronome FM-09, Mute/Solo FM-08)
    virtual void sendEvent(const MidiEvent& event) = 0; // FM-07
    virtual void sendNoteOn(int channel, int note, int velocity) = 0;
    virtual void sendNoteOff(int channel, int note) = 0;
    virtual void sendControlChange(int channel, int ccNumber, int value) = 0; // For Sustain FM-05

    // Device Management
    virtual void pollEvents() = 0; // Or use callbacks if library supports
    virtual bool isInputDeviceConnected() const = 0;
    virtual bool isOutputDeviceConnected() const = 0;
    // Add handling for device connection/disconnection events (FM-12, NF-REL-01)
};
```

*   **Functionality:** Manages detection and selection of MIDI input/output devices (FM-01, FM-02). Processes incoming MIDI events (Note On/Off, Velocity, CC - FM-03, FM-04, FM-05) for note input and control. Sends MIDI events for score playback (FM-07), including metronome clicks (FM-09) and respecting mute/solo states (FM-08). Supports channel mapping (FM-11).
*   **Architecture:** Uses a platform-specific MIDI library (e.g., RtMidi, ALSA, CoreMIDI via QtMultimedia). Employs a dedicated thread and lock-free queue (Section 2.3 in original spec, now implied here) for processing incoming events to minimize latency.
*   **Performance:** Critical focus on minimizing input-to-output latency (<= 10ms target - FM-13, NF-PERF-01). Event batching might be used for output if beneficial.
*   **Reliability:** Implements graceful handling of device connection/disconnection (FM-12, NF-REL-01), potentially with automatic reconnection attempts and user notifications.

## 3. Feature Implementation Details

### 3.1 Preset System (SRS V4: 3.1.6)
*   Manages loading/saving of presets from/to JSON files (FD-02, FP-04, FP-05). Schema includes metadata, rule enablement status, and parameter values for each configurable rule (FR-09).
*   Provides UI for selecting predefined presets and managing custom ones (create, rename, delete - FP-01, FP-03, FP-06, FP-07, FP-08).
*   Integrates with Rule Engine for applying presets (`RuleEngine::loadRuleSet`) and checking conflicts (`RuleEngine::checkRuleConflicts`) (FP-02, FP-09).
*   Preset configurations are persistent (FP-10).
*   (Low Priority: Hot-reloading FP-11).

### 3.2 Voice Analysis (SRS V4: 3.1.7 - Analysis aspects)
*   Analyzes melodic contours, ranges, and intervals within voices (FF-06).
*   Analyzes harmonic intervals between voice pairs (FF-07).
*   (Low Priority: Rhythmic analysis FF-08, Role identification FF-12, Relationship analysis FF-13).
*   Uses pattern matching algorithms (Section 5.2) and statistical analysis.
*   Results are provided to the Feedback/Analysis System for display.
*   Runs primarily in background threads to avoid blocking UI.

### 3.3 Feedback/Analysis System (SRS V4: 3.1.7)
*   Consolidates `ValidationResult` from Rule Engine and analysis data from Voice Analysis system.
*   Presents information clearly in the dedicated Feedback Area UI panel (FF-01, FF-02, FF-03).
*   Allows navigation from feedback items (e.g., violations) to the corresponding location in the Score Viewport (FF-04).
*   Updates in near real-time as the score changes (FF-05).
*   (Medium Priority: Provides suggestions for improvement FF-09).
*   (Low Priority: Historical analysis FF-10).

### 3.4 Command/Undo System (SRS V4: FN-10)
*   Implements the Command pattern. Each user action that modifies the score state (add/delete/move note, change time signature, etc.) is encapsulated in a command object implementing `ICommand` (with `execute()` and `undo()` methods).
*   Maintains a stack of executed commands for undo and a stack for redo.
*   Ensures reliable state changes and rollbacks (NF-REL-04).
*   Integrates with UI actions (menu items, keyboard shortcuts FU-19).

## 4. Performance Optimizations

Techniques employed to meet performance requirements defined in `SRS_V4.md` (NF-PERF-xx).

### 4.1 Rule Validation Optimization (Target: NF-PERF-04 < 50ms)
*   Incremental validation using dirty regions (only re-validate affected areas).
*   Rule caching based on voice positions and calculated intervals.
*   Parallel validation for independent rules where feasible.
*   Priority-based rule execution (e.g., check cheaper rules first).

### 4.2 Memory Management Optimization (Target: NF-PERF-03 <= 512MB)
*   Object pooling for frequently created/destroyed objects (e.g., temporary analysis objects).
*   Copy-on-write semantics for core Domain Model objects (Score, Voice) to minimize copying during modifications (supports Undo).
*   Smart pointer usage (`std::shared_ptr`, `std::unique_ptr`) for managing resource lifetimes.
*   Cache size limits based on available system memory or configurable settings.
*   Efficient data structures (Section 4) minimizing overhead.

### 4.3 Real-time Processing & Responsiveness (Targets: NF-PERF-01, NF-PERF-05, NF-PERF-06, NF-PERF-07)
*   Dedicated threads for time-sensitive tasks (MIDI I/O - Section 2.4).
*   Thread pool for background tasks (e.g., Voice Analysis - Section 3.2, complex Solution Generation - Section 2.2).
*   Event batching for UI updates (e.g., collecting multiple score changes before triggering a single redraw/validation).
*   Lazy evaluation/loading where appropriate (e.g., loading exercise details on demand, delaying complex analysis).
*   Asynchronous operations for file I/O (Persistence - Section 1) to avoid blocking the main thread (supports NF-PERF-05, NF-PERF-06).

### 4.4 Viewport Rendering & Navigation Optimization (Targets: NF-PERF-02 >= 60 FPS, NF-PERF-08 Smooth Expansion)
*   Lazy grid expansion with configurable thresholds (prevents allocating huge grids upfront).
*   Coordinate transformation caching (musical <-> screen) with LRU eviction.
*   Viewport-based content loading/streaming (only process/render data within or near the visible area).
*   Optimized drawing routines (e.g., using `QPainter` efficiently, potentially GPU acceleration via Qt Quick/OpenGL if needed later).
*   Event batching for grid updates (collecting changes within a frame).
*   Background loading/prefetching of adjacent grid regions during navigation.
*   Level-of-Detail (LOD) rendering (e.g., simplifying note display at high zoom-out levels).
*   Spatial indexing (e.g., Quadtree, R-tree) for efficient lookup of notes within the visible viewport bounds.

## 4. Data Structures

### 4.1 Domain Model Objects (Immutable)
*   **Note:** Represents a single musical note (pitch, start time, duration, velocity, voice ID). Immutable after creation.
*   **TimeSignature:** Represents the time signature (beats per measure, beat unit).
*   **KeySignature:** Represents the key signature (tonic, mode).
*   **Score:** Represents the complete musical piece. Contains collections of Notes, TimeSignatures, KeySignatures, and Voice metadata. Uses efficient storage (e.g., `std::vector` per voice, potentially indexed by time). Designed for copy-on-write semantics or similar to support undo/redo and efficient updates.
*   **Interval:** Represents the distance between two pitches.
*   **Position:** Represents a point in musical time (e.g., measure, beat, offset).
*   **MusicalCoordinate:** Combines Position and Pitch.

### 4.2 Score Storage & Indexing
```cpp
// Efficient score storage and querying within the Score object
class Score {
    // ... other members ...
    struct Voice {
        std::vector<Note> notes; // Sorted by start time
        // Potentially spatial index (e.g., interval tree) for faster range queries
    };
    std::vector<Voice> voices;
    std::map<Position, TimeSignature> timeSignatures; // Time-indexed
    std::map<Position, KeySignature> keySignatures;   // Time-indexed

    // Methods for efficient querying (used by Rule Engine, Viewport, etc.)
    std::vector<Note*> getNotesInRange(const TimeRange& timeRange, const PitchRange& pitchRange, int voiceId = -1);
    std::vector<Note*> getNotesAtTime(Position time, int voiceId = -1);
    TimeSignature getTimeSignatureAt(Position time);
    // ... etc ...
};
```
*   Focus on efficient querying needed by validation and rendering. Notes within voices are typically sorted by start time. Additional indexing (e.g., interval trees, time-based maps) may be used for faster range lookups.

### 4.3 Rule & Preset Data
```cpp
// Parameters for a specific rule instance
using RuleParameters = std::variant<int, float, std::string, std::vector<int>, ...>; // Or std::any, or specific structs

// Represents a configured rule within a preset
struct PresetRuleConfig {
    RuleId ruleId;
    bool isEnabled;
    std::map<std::string, RuleParameters> parameters; // Parameter name -> value
};

// Represents a complete preset (serialized to JSON, see Section 1)
struct Preset {
    std::string id;
    std::string name;
    std::string description;
    std::vector<PresetRuleConfig> ruleConfigs;
    // Potentially other exercise context settings (e.g., default time signature)
};

// Internal Rule Engine storage might use maps for fast lookup
class RuleRegistry {
    std::unordered_map<RuleId, std::shared_ptr<IRule>> registeredPlugins;
    // Stores the currently active Preset configuration for validation
    Preset activePreset;
};
```
*   Presets define rule enablement and parameterization (FP-xx). Stored as JSON externally. Internally, the Rule Engine maintains the active preset configuration.

### 4.4 Exercise File Format (`.mtx`) (FD-06)
*   Format TBD (JSON preferred for readability, Binary for performance/size).
*   **Contents:**
    *   `version`: Format version number.
    *   `metadata`: { `title`, `description`, `goals`, `difficulty`, `category` } (FE-08)
    *   `score`: Embedded Score data (Notes, Voices, Time/Key Signatures).
    *   `presetId`: Reference to the associated Preset ID (or embedded Preset data).
    *   `cantusFirmusVoiceId`: Optional ID of the voice containing the fixed melody.
    *   (Future: `userProgress`: { `attempts`, `lastState`, `violations` })

### 4.5 Viewport Data Structures
```cpp
// Represents the current state of the viewport
struct ViewportState {
    Rectangle visibleMusicalBounds; // Time and Pitch range visible
    float timeScale; // Horizontal zoom
    float pitchScale; // Vertical zoom
    // ... potentially scroll offsets ...
};

// Internal grid management (details omitted, similar to V3 spec but optimized)
// Focuses on mapping musical coordinates to screen coordinates efficiently,
// managing rendering caches, and handling lazy loading/expansion.
// Uses caching (TransformCache) and spatial indexing for visible notes.
```
*   Manages the mapping between musical coordinates and screen space based on zoom/pan state. Caching is crucial for performance (NF-PERF-02, NF-PERF-08).

### 4.6 Command/Undo Data Structures (FN-10)
```cpp
// Base interface for undoable commands
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0; // For UI display
};

// Example command
class AddNoteCommand : public ICommand {
    Score& targetScore; // Reference to the score being modified
    Note noteToAdd;
public:
    // Constructor stores necessary context
    void execute() override { /* Add noteToAdd to targetScore */ }
    void undo() override { /* Remove noteToAdd from targetScore */ }
    std::string getDescription() const override { return "Add Note"; }
};

// Manages the undo/redo stacks
class CommandHistory {
    std::stack<std::shared_ptr<ICommand>> undoStack;
    std::stack<std::shared_ptr<ICommand>> redoStack;
public:
    void executeCommand(std::shared_ptr<ICommand> command);
    void undoLastCommand();
    void redoLastCommand();
    void clear();
    bool canUndo() const;
    bool canRedo() const;
};
```
*   Uses the Command pattern. Concrete command classes (e.g., `AddNoteCommand`, `DeleteNoteCommand`, `MoveNoteCommand`) encapsulate state changes and their reversal. `CommandHistory` manages the undo/redo stacks.

## 5. Critical Algorithms

### 5.1 Solution Generation (A*)
```cpp
// A* search implementation for finding valid note sequences
class SolutionFinder {
    struct SearchNode {
        Score scoreState; // Current state of the score fragment
        Position currentTime;
        float costG; // Cost from start to current node
        float costH; // Heuristic estimate to goal
        std::shared_ptr<SearchNode> parent; // To reconstruct path

        float getCostF() const { return costG + costH; }
        // Comparison operator for priority queue
        bool operator>(const SearchNode& other) const { return getCostF() > other.getCostF(); }
    };

    // Heuristic function: Estimates remaining cost (e.g., based on distance to end, number of remaining notes, estimated violation count)
    float heuristic(const Score& score, Position currentTime);

    // Generates valid successor nodes (next possible notes) based on Rule Engine validation
    std::vector<SearchNode> generateSuccessors(const SearchNode& node, const RuleEngine& validator);

    // A* search algorithm using priority queue
    std::optional<Score> findSolution(const ExerciseContext& context, const RuleEngine& validator);
};
```
*   Uses A* search to explore the space of possible note additions.
*   Nodes represent partial score states.
*   Successor generation involves finding notes that are valid according to the `RuleEngine` at the current position.
*   Heuristics guide the search towards promising solutions (FS-04, FS-06). Pruning strategies may be needed for performance (FS-08).

### 5.2 Pattern Recognition (Voice Analysis)
```cpp
// Algorithms for analyzing voice characteristics (FF-06, FF-07, FF-12, FF-13)
class VoiceAnalyzer {
    // Calculates melodic contour features (e.g., number of leaps, steps, direction changes)
    MelodicContourFeatures analyzeMelodicContour(const Voice& voice);

    // Calculates voice range
    PitchRange analyzeVoiceRange(const Voice& voice);

    // Analyzes harmonic intervals between two voices over time
    HarmonicAnalysisResult analyzeHarmonicIntervals(const Voice& voice1, const Voice& voice2);

    // (Future/Low Priority) Identifies voice roles based on range and contour
    VoiceRole identifyVoiceRole(const Voice& voice);

    // (Future/Low Priority) Detects relationships like imitation using pattern matching (e.g., KMP, Edit Distance)
    std::vector<PatternMatch> findImitation(const Voice& voice1, const Voice& voice2);
};
```
*   Uses algorithms for interval calculation, statistical analysis of melodic movement, and potentially string matching algorithms (KMP, Levenshtein distance) for pattern detection like imitation.

## 6. Error Handling

### 6.1 Error Categories & Reporting
*   **MIDI Device Errors:** Connection loss, configuration issues. (FM-12, NF-REL-01)
*   **File I/O Errors:** Corrupted files (preset, exercise), permissions issues. (NF-REL-02)
*   **Validation Errors:** Internal Rule Engine errors (distinct from score violations).
*   **Resource Exhaustion:** Memory allocation failures, thread limits.
*   **State Consistency Errors:** Unexpected application state, data corruption.
*   **Configuration Errors:** Invalid preset parameters, rule conflicts detected at load time. (FR-11)
*   **User Input Errors:** Invalid data entered in dialogs, etc.

*   **Reporting:** Errors are logged using the Structured Logging system (Section 9.3, FDV-06). User-facing errors are presented via dialogs or status bar messages, avoiding technical jargon where possible (NF-USAB-08).

### 6.2 Recovery Strategies
*   **MIDI:** Attempt automatic reconnection (FM-12), notify user if persistent.
*   **File I/O:** Report error to user, prevent crash, potentially offer recovery options (e.g., load backup).
*   **State:** Use transactions or Command pattern (Section 3.4) to ensure atomic updates where possible. Implement sanity checks.
*   **Resource:** Graceful degradation (e.g., disable optional features), notify user.
*   **General:** Isolate faulty components where possible. Provide clear error messages to aid debugging.

## 7. Testing Strategy (SRS V4: NF-SUP-05, NF-SUP-06, FDV-xx)

A multi-layered testing strategy is employed, focusing on automation and supporting agent-based development/testing.

### 7.1 Unit Tests (NF-SUP-05)
*   **Scope:** Individual classes and functions in isolation.
*   **Tools:** C++ testing framework (e.g., Google Test, Catch2).
*   **Focus:**
    *   Domain Model logic (interval calculation, time representation).
    *   Individual Rule validation logic (using mocked Scores).
    *   Algorithm correctness (A*, pattern matching).
    *   Data structure operations.
    *   Utility functions.
*   **Execution:** Run frequently during development and in CI pipelines.

### 7.2 Integration Tests (NF-SUP-05)
*   **Scope:** Interactions between components.
*   **Tools:** C++ testing framework, potentially using the headless/CLI mode (FDV-07).
*   **Focus:**
    *   Rule Engine validating a complete Score.
    *   Preset System loading and applying rules to the Engine.
    *   Solution Generator interacting with the Rule Engine.
    *   Persistence Service saving/loading exercises and presets.
    *   Command History executing and undoing sequences of actions.
    *   MIDI System processing simulated events.
*   **Execution:** Run in CI pipelines after unit tests pass.

### 7.3 End-to-End (E2E) / System Tests
*   **Scope:** Testing the application as a whole, simulating user workflows.
*   **Tools:** Primarily via the Dev/Testing Interface (CLI commands, structured log parsing - Section 9). GUI automation tools (e.g., Squish, LDTP) might be used sparingly for critical UI workflows if CLI coverage is insufficient.
*   **Focus:**
    *   Loading an exercise, adding notes via CLI, validating, getting feedback.
    *   Loading presets via CLI, checking active rules.
    *   Generating hints/solutions via CLI.
    *   Saving/loading application state via files.
    *   Testing core workflows identified from user scenarios.
*   **Execution:** Run less frequently (e.g., nightly builds, pre-release) in CI/CD pipelines. Crucial for agent-based testing.

### 7.4 Performance Tests
*   **Scope:** Measuring performance characteristics against SRS targets (NF-PERF-xx).
*   **Tools:** Benchmarking frameworks, profiling tools (e.g., Valgrind, Perf), custom test harnesses using the CLI.
*   **Focus:**
    *   MIDI latency (NF-PERF-01).
    *   UI responsiveness (simulated via CLI interactions where possible, NF-PERF-02).
    *   Rule validation time (NF-PERF-04).
    *   Solution generation time (NF-PERF-07).
    *   Memory usage under load (NF-PERF-03).
    *   File loading times (NF-PERF-06).
*   **Execution:** Run periodically and before releases to track regressions.

### 7.5 Testability Support (NF-SUP-06, Section 9.4)
*   Code designed with testability in mind (clear interfaces, dependency injection).
*   Dev/Testing Interface provides crucial hooks for automated E2E and integration testing without relying solely on fragile GUI automation.
*   Structured logs provide detailed state information for debugging automated tests.

## 8. Deployment Considerations

### 8.1 Target Platforms & Dependencies (SRS V4: 2.4, NF-COMP-xx)
*   **Operating Systems:**
    *   Windows 10/11 (64-bit) (NF-COMP-01)
    *   Linux (Ubuntu 20.04+, Debian 11+ recommended, 64-bit) (NF-COMP-02)
    *   macOS: Secondary target, support TBD.
*   **Core Dependencies:**
    *   C++17 compliant compiler (e.g., GCC, Clang, MSVC).
    *   CMake (Version TBD) for build system.
    *   Qt 6 Framework (Specific minimum version TBD, NF-COMP-04).
    *   ALSA library (Linux for MIDI/Audio).
    *   Potentially RtMidi or similar cross-platform MIDI library (if QtMultimedia MIDI is insufficient).
*   **Build Process:** Standard CMake workflow (configure, build).

### 8.2 Resource Management
*   Dynamic resource loading (e.g., exercise files, presets).
*   Memory usage monitoring during development and testing against targets (NF-PERF-03).
*   Background task scheduling using thread pools (Section 3.3).
*   Cache size adjustments based on available memory.

### 8.3 Packaging & Distribution
*   Windows: Installer (e.g., NSIS, Inno Setup) bundling Qt libraries.
*   Linux: Potentially AppImage, Flatpak, or Debian package (.deb) bundling dependencies.
*   Ensure necessary runtime libraries (VC++ Redistributable on Windows, standard C++ libs on Linux) are handled.

### 8.4 Error Monitoring & Reporting
*   Utilize the Structured Logging system (Section 9.3) for capturing runtime errors and performance metrics in deployment.
*   Consider optional opt-in telemetry for reporting crashes or critical errors (Future Consideration).


## 9. Development and Testing Support Interface

This interface provides mechanisms for automated testing, debugging, and interaction by external agents or scripts, fulfilling requirements FDV-01 to FDV-08.

### 9.1 Command-Line Interface (CLI) / Console Mode (FDV-01, FDV-07)

*   The application core logic (Domain Model, Rule Engine, Persistence, etc.) shall be runnable headless, without initializing the GUI.
*   A CLI executable or mode will be provided.
*   Commands will interact with the Application Core/State Management layer.
*   Input/Output will use standard streams (stdin/stdout/stderr).

### 9.2 CLI Commands (FDV-02 to FDV-05, FDV-08)

*   `load_exercise <path_to_mtx>`: Loads an exercise file.
*   `load_preset <path_to_json>`: Loads and applies a preset file.
*   `get_score_summary`: Outputs basic score info (voices, measures, notes).
*   `add_note <voice_index> <pitch_midi> <start_beat> <duration_beats> <velocity>`: Adds a note.
*   `delete_note <voice_index> <pitch_midi> <start_beat>`: Deletes a note (requires precise identification).
*   `move_note <voice_index> <pitch_midi> <start_beat> <new_start_beat> <new_pitch_midi>`: Modifies a note.
*   `validate_score`: Triggers rule validation.
*   `get_feedback`: Retrieves validation results (violations) in a structured format (JSON).
*   `get_state`: Retrieves key application state info (loaded exercise, active preset) in JSON.
*   `generate_solution`: Attempts to generate a solution (FS-06).
*   `get_solution`: Retrieves the generated solution in a structured format.
*   `quit`/`exit`: Terminates the application.

### 9.3 Structured Logging (FDV-06)

*   Logs will be output in JSON format to stderr or a configurable file.
*   Each log entry will include timestamp, log level (DEBUG, INFO, WARN, ERROR), component source (e.g., "RuleEngine", "MIDI"), and a structured message payload.
*   Payloads will contain relevant data (e.g., MIDI event details, rule violation specifics, state changes).
*   This facilitates automated parsing and analysis by testing frameworks or agents.

### 9.4 Testability Design (NF-SUP-05, NF-SUP-06)

*   Core components will have clear, minimal interfaces.
*   Dependency injection will be used where practical to allow mocking.
*   State changes will be predictable and primarily driven by explicit commands or events.
*   Minimize side effects in core logic functions.


### 7.2 Performance Tests
- Rule validation benchmarks
- Solution generation timing
- Memory usage patterns
- UI responsiveness metrics

## 8. Deployment Considerations

### 8.1 Resource Management
- Dynamic resource loading
- Memory usage monitoring
- Background task scheduling
- Cache size adjustment

### 8.2 Error Monitoring
- Error logging system
- Performance metrics collection
- User interaction tracking
- System health monitoring