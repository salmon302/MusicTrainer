# Domain Integration Guide

## 1. Core Integration Patterns

### 1.1 Event-Based Communication
```cpp
// Example: Publishing state changes from a component
class ScoreView {
    void onViewportChanged() {
        auto state = ViewportState{
            m_position.x(),
            m_position.y(),
            m_zoomLevel,
            m_preserveExpansion
        };
        m_stateCoordinator->updateViewportState(state);
    }
};

// Example: Subscribing to state changes
class MainWindow {
    void connectSignals() {
        m_stateCoordinator->subscribeToState<ViewportState>(
            StateType::VIEWPORT_CHANGE,
            [this](const auto& state) { onViewportChanged(state); }
        );
    }
};
```

### 1.2 State Synchronization
```cpp
// Example: Component state initialization
class TransportControls {
    void initialize() {
        // Get last known state
        if (auto state = m_stateCoordinator->getLastState<PlaybackState>(
                StateType::PLAYBACK_STATE_CHANGE)) {
            // Initialize UI from state
            m_playButton->setChecked(state->isPlaying);
            m_tempoSpinner->setValue(state->tempo);
            m_metronomeButton->setChecked(state->metronomeEnabled);
        }
    }
};
```

## 2. Implementation Guidelines

### 2.1 Event Flow Best Practices
1. Use typed events for compile-time safety
2. Include source identification for debugging
3. Handle state validation at boundaries
4. Implement proper error propagation
5. Consider state versioning for conflict resolution
6. Use coalescing for rapid updates

### 2.2 State Management
1. Define clear state ownership
2. Implement atomic state transitions
3. Validate state consistency
4. Handle partial state updates
5. Maintain state history when needed
6. Implement recovery mechanisms

## 3. Error Handling Integration

### 3.1 Error Propagation Pattern
```cpp
// Example: Error handling in state updates
class ScoreView {
    void updateState() {
        try {
            // Create state backup
            auto backup = createStateBackup();
            
            // Attempt state update
            applyStateChange();
            validateState();
            
            // Publish if valid
            publishStateChange();
        } catch (const StateValidationError& e) {
            // Log error
            ErrorLogger::log(e);
            
            // Attempt recovery
            if (!recoverFromError()) {
                // Restore backup on failure
                restoreFromBackup();
            }
            
            // Notify user
            notifyUserOfError(e);
        }
    }
};
```

### 3.2 Recovery Strategy
1. Validate preconditions
2. Create state backups
3. Apply changes atomically
4. Validate postconditions
5. Handle recovery actions
6. Notify affected components

## 4. Performance Optimization

### 4.1 Update Coalescing
```cpp
// Example: Coalescing rapid viewport updates
class ViewportManager {
    void scheduleUpdate() {
        if (!m_updatePending) {
            m_updatePending = true;
            QTimer::singleShot(16, this, [this]() {
                processScheduledUpdate();
                m_updatePending = false;
            });
        }
    }
};
```

### 4.2 State Caching
```cpp
// Example: Caching validated states
class StateManager {
    std::optional<State> getCachedState(const StateId& id) {
        auto it = m_stateCache.find(id);
        if (it != m_stateCache.end() && !isCacheExpired(it->second)) {
            return it->second.state;
        }
        return std::nullopt;
    }
};
```

## 5. Testing Strategy

### 5.1 Component Integration Tests
```cpp
// Example: Testing state synchronization
TEST_F(IntegrationTest, StateSync) {
    // Setup components
    auto eventBus = EventBus::create();
    auto coordinator = GuiStateCoordinator::create(eventBus);
    auto component = TestComponent::create(coordinator);
    
    // Test state propagation
    auto testState = ViewportState{1.0f, 2.0f, 1.5f, true};
    component->updateViewportState(testState);
    
    // Verify state received
    auto receivedState = coordinator->getLastState<ViewportState>(
        StateType::VIEWPORT_CHANGE);
    ASSERT_TRUE(receivedState);
    EXPECT_EQ(receivedState->x, testState.x);
    EXPECT_EQ(receivedState->y, testState.y);
}
```

### 5.2 Performance Tests
```cpp
// Example: Testing update coalescing
TEST_F(PerformanceTest, UpdateCoalescing) {
    auto component = createTestComponent();
    
    // Generate rapid updates
    for (int i = 0; i < 100; ++i) {
        component->updateState();
    }
    
    // Verify coalesced updates
    EXPECT_LE(getEventCount(), 10);
    EXPECT_LE(getUpdateTime(), 16ms);
}
```

## 6. Common Integration Scenarios

### 6.1 MIDI Input Integration
```cpp
// Example: MIDI event handling
class MidiComponent {
    void handleMidiEvent(const MidiEvent& event) {
        // Update internal state
        updateState(event);
        
        // Publish state change
        auto state = MidiDeviceState{
            event.port,
            m_outputPort,
            m_midiThrough,
            m_latency
        };
        m_stateCoordinator->updateMidiDeviceState(state);
    }
};
```

### 6.2 Score Display Integration
```cpp
// Example: Score view updates
class ScoreView {
    void handleScoreUpdate(const Score& score) {
        // Update display state
        updateDisplay(score);
        
        // Update selection if needed
        if (hasSelection()) {
            updateSelectionState();
        }
        
        // Publish display state
        publishDisplayState();
    }
};
```

## 7. Documentation Standards

### 7.1 Component Documentation
```cpp
/**
 * @brief Documents a component's integration points
 *
 * @par State Management
 * - Publishes: ViewportState, SelectionState
 * - Subscribes to: ScoreState, PlaybackState
 *
 * @par Error Handling
 * - Validates state changes
 * - Implements recovery for invalid states
 * - Propagates errors to UI
 *
 * @par Performance
 * - Implements update coalescing
 * - Caches validated states
 * - Uses batched updates
 */
```

### 7.2 Integration Testing
```cpp
/**
 * @brief Documents integration test coverage
 *
 * @test Component Integration
 * - State synchronization
 * - Event propagation
 * - Error handling
 *
 * @test Performance
 * - Update coalescing
 * - State caching
 * - Resource usage
 */
```