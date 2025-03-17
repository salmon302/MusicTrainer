# State Management Documentation

## 1. Core State Architecture

### 1.1 State Components
```cpp
// Core state components
struct SystemState {
    // Score state
    struct ScoreState {
        std::shared_ptr<const Score> currentScore;
        std::vector<ScoreEvent> eventHistory;
        ValidationState validationState;
    };
    
    // Viewport state
    struct ViewportState {
        Rectangle visibleArea;
        float zoomLevel;
        bool preserveOctaveExpansion;
        QPointF scrollPosition;
        int pitchBase;
    };
    
    // Exercise state
    struct ExerciseState {
        ExerciseId currentExercise;
        std::vector<Solution> attempts;
        ProgressMetrics progress;
        ValidationState validation;
    };
};
```

### 1.2 GUI State Integration

#### Component States
```cpp
// Display state
struct ScoreDisplayState {
    bool showMeasureNumbers;
    bool showKeySignature;
    bool showVoiceLabels;
    int fontSize;
};

// Selection state
struct SelectionState {
    int voiceIndex;
    int measureIndex;
    int noteIndex;
};

// Playback state
struct PlaybackState {
    bool isPlaying;
    bool isRecording;
    int tempo;
    bool metronomeEnabled;
};

// MIDI device state
struct MidiDeviceState {
    int inputDeviceIndex;
    int outputDeviceIndex;
    bool midiThrough;
    int latency;
};
```

### 1.3 State Management Patterns

#### Event-Based State Updates
- State changes are propagated through the GuiStateEvent system
- Events include source tracking for debugging and validation
- State transitions are atomic and versioned
- Update coalescing for performance optimization

#### State Validation and Recovery
```cpp
class StateManager {
    // Validation
    bool validateState() const;
    void enforceStateConstraints();
    
    // Recovery
    void createStateBackup();
    bool restoreFromBackup();
    void recoverFromInvalidState();
    
    // State persistence
    void saveState();
    bool loadState();
    void emitStateChanges();
};
```

## 2. Integration Architecture

### 2.1 Component Integration
- GuiStateCoordinator acts as central state manager
- Components subscribe to relevant state changes
- State updates are published through EventBus
- State recovery mechanisms handle invalid states

### 2.2 State Flow Pattern
```
[Component] -> GuiStateEvent -> EventBus -> GuiStateHandler
                                           -> Update State
                                           -> Notify Subscribers
                                           -> Persist Changes
```

### 2.3 Error Handling
- State validation before updates
- Automatic state recovery on validation failure
- Backup/restore mechanisms for critical state changes
- Error propagation through event system

## 3. Implementation Guidelines

### 3.1 Adding New State Types
1. Define state structure in GuiStateEvent
2. Add state type to StateType enum
3. Update StateVariant with new type
4. Implement validation in StateManager
5. Add persistence methods
6. Update documentation

### 3.2 State Update Best Practices
1. Use GuiStateCoordinator for all state changes
2. Implement proper validation
3. Include source identification
4. Handle recovery scenarios
5. Update through event system
6. Coalesce rapid updates

### 3.3 Error Recovery Process
1. State validation failure detected
2. Attempt automatic recovery
3. Restore from backup if needed
4. Notify components of state change
5. Log recovery details
6. Update UI accordingly

## 4. Performance Considerations

### 4.1 Update Coalescing
- Batch rapid state changes
- Debounce viewport updates
- Optimize event processing
- Cache frequently accessed states

### 4.2 Memory Management
- Use shared pointers for large states
- Clear old state history
- Implement state compression
- Manage backup cleanup

## 5. Testing and Validation

### 5.1 State Tests
- Validate state transitions
- Test recovery mechanisms
- Verify persistence
- Check concurrency handling

### 5.2 Integration Tests
- Component interaction validation
- Event propagation verification
- Error handling scenarios
- Performance benchmarks