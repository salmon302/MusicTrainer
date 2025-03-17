# State Management Implementation Guide

## 1. State Types Implementation

### 1.1 Defining New State Types
```cpp
// Example: Adding a new state type
namespace music::events {
    // 1. Add to StateType enum
    enum class GuiStateEvent::StateType {
        // ...existing types...
        MY_NEW_STATE_CHANGE
    };

    // 2. Define state structure
    struct GuiStateEvent::MyNewState {
        int someValue;
        bool someFlag;
        std::string identifier;
    };

    // 3. Add to StateVariant
    using StateVariant = std::variant<
        // ...existing types...
        MyNewState
    >;
}
```

### 1.2 State Update Flow
```cpp
// Example: Implementing state updates
class MyComponent {
    void updateComponentState() {
        // Create state
        auto state = MyNewState{
            .someValue = calculateValue(),
            .someFlag = checkCondition(),
            .identifier = generateId()
        };

        // Validate
        if (!validateState(state)) {
            handleInvalidState();
            return;
        }

        // Update via coordinator
        m_stateCoordinator->updateState(
            StateType::MY_NEW_STATE_CHANGE,
            state,
            "MyComponent"
        );
    }
};
```

## 2. State Validation

### 2.1 Validation Implementation
```cpp
// Example: State validation
class StateValidator {
    ValidationResult validateState(const MyNewState& state) {
        ValidationResult result;
        
        // Value range checks
        if (state.someValue < 0 || state.someValue > 100) {
            result.addError("Value out of range");
        }

        // Flag consistency
        if (state.someFlag && state.someValue == 0) {
            result.addError("Invalid flag state");
        }

        // Required fields
        if (state.identifier.empty()) {
            result.addError("Missing identifier");
        }

        return result;
    }
};
```

### 2.2 Recovery Implementation
```cpp
// Example: State recovery
class StateRecovery {
    bool recoverFromInvalidState(MyNewState& state) {
        // Attempt auto-correction
        if (state.someValue < 0) {
            state.someValue = 0;
            return true;
        }
        if (state.someValue > 100) {
            state.someValue = 100;
            return true;
        }

        // Clear inconsistent flags
        if (state.someFlag && state.someValue == 0) {
            state.someFlag = false;
            return true;
        }

        return false;
    }
};
```

## 3. Event Handling

### 3.1 Event Handler Implementation
```cpp
// Example: Component event handler
class MyComponentHandler {
    void handleStateEvent(const GuiStateEvent& event) {
        if (event.getStateType() == StateType::MY_NEW_STATE_CHANGE) {
            if (const auto* state = std::get_if<MyNewState>(&event.getState())) {
                // Handle state update
                updateFromState(*state);
                
                // Update UI if needed
                updateUI();
                
                // Notify children
                notifyStateChange(*state);
            }
        }
    }
};
```

### 3.2 State Change Subscription
```cpp
// Example: State change subscription
class MyComponent {
    void initializeStateHandling() {
        // Subscribe to specific state type
        m_stateCoordinator->subscribeToState<MyNewState>(
            StateType::MY_NEW_STATE_CHANGE,
            [this](const auto& state) {
                // Store state
                m_currentState = state;
                
                // Update view
                updateView();
                
                // Emit signals
                Q_EMIT stateChanged(state);
            }
        );

        // Initialize from last known state
        if (auto state = m_stateCoordinator->getLastState<MyNewState>(
                StateType::MY_NEW_STATE_CHANGE)) {
            updateFromState(*state);
        }
    }
};
```

## 4. Performance Patterns

### 4.1 State Change Batching
```cpp
// Example: Batching state updates
class BatchedStateManager {
    void queueStateUpdate(const MyNewState& state) {
        // Add to batch
        m_pendingUpdates.push_back(state);
        
        // Schedule processing if needed
        if (!m_updateScheduled) {
            m_updateScheduled = true;
            QTimer::singleShot(0, this, &BatchedStateManager::processPendingUpdates);
        }
    }

    void processPendingUpdates() {
        // Combine updates
        auto combinedState = combineUpdates(m_pendingUpdates);
        
        // Publish single update
        publishStateUpdate(combinedState);
        
        // Clear batch
        m_pendingUpdates.clear();
        m_updateScheduled = false;
    }
};
```

### 4.2 Update Coalescing
```cpp
// Example: Coalescing rapid updates
class CoalescingManager {
    void scheduleStateUpdate(const MyNewState& state) {
        // Store latest state
        m_pendingState = state;
        
        // Reset or start coalescing timer
        if (!m_coalesceTimer.isActive()) {
            m_coalesceTimer.start(16); // ~60 FPS
        }
    }

    void onCoalesceTimerTimeout() {
        if (m_pendingState) {
            // Publish coalesced update
            publishStateUpdate(*m_pendingState);
            m_pendingState.reset();
        }
    }
};
```

## 5. Testing Patterns

### 5.1 State Change Testing
```cpp
// Example: State change test
class StateChangeTest : public ::testing::Test {
    void TestStateTransition() {
        // Setup
        auto coordinator = createTestCoordinator();
        auto component = createTestComponent(coordinator);
        bool stateReceived = false;
        
        // Subscribe to changes
        coordinator->subscribeToState<MyNewState>(
            StateType::MY_NEW_STATE_CHANGE,
            [&](const auto& state) {
                stateReceived = true;
                EXPECT_EQ(state.someValue, 42);
                EXPECT_TRUE(state.someFlag);
            }
        );
        
        // Trigger state change
        component->updateSomeValue(42);
        
        // Verify
        EXPECT_TRUE(stateReceived);
    }
};
```

### 5.2 Recovery Testing
```cpp
// Example: Recovery test
class RecoveryTest : public ::testing::Test {
    void TestStateRecovery() {
        // Setup invalid state
        auto state = MyNewState{
            .someValue = -1,
            .someFlag = true,
            .identifier = ""
        };
        
        // Attempt recovery
        auto recovery = StateRecovery();
        bool recovered = recovery.recoverFromInvalidState(state);
        
        // Verify recovery
        EXPECT_TRUE(recovered);
        EXPECT_GE(state.someValue, 0);
        EXPECT_FALSE(state.someFlag);
        EXPECT_FALSE(state.identifier.empty());
    }
};
```

## 6. Implementation Checklist

When adding a new state type:

1. State Definition
   - [ ] Add StateType enum value
   - [ ] Define state structure
   - [ ] Add to StateVariant
   - [ ] Document state fields

2. Validation
   - [ ] Implement state validation
   - [ ] Define recovery strategies
   - [ ] Add error handling
   - [ ] Test validation cases

3. Event Handling
   - [ ] Add event handler
   - [ ] Implement state updates
   - [ ] Add change notifications
   - [ ] Test event flow

4. Performance
   - [ ] Consider update batching
   - [ ] Implement coalescing
   - [ ] Add caching if needed
   - [ ] Test performance

5. Documentation
   - [ ] Document state purpose
   - [ ] List dependencies
   - [ ] Describe validation rules
   - [ ] Add usage examples