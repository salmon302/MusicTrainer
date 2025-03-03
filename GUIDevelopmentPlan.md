# GUI Development Plan for MusicTrainer

## 1. Overview
This document outlines the development plan for the graphical user interface of the MusicTrainer application. The GUI will be built using Qt6 and will follow the MVVM (Model-View-ViewModel) pattern with reactive bindings as specified in ADR-008. This plan is designed to integrate with the existing domain model and infrastructure components.

## 2. Technology Stack
- **GUI Framework**: Qt6 (Qt6::Core, Qt6::Widgets)
- **Architecture Pattern**: MVVM with reactive bindings
- **Platform Support**: Cross-platform (Windows 10/11 as primary target, Linux support)
- **MIDI Library**: RtMidi integration (via existing RtMidiAdapter)
- **Performance Targets**: 
  - Maximum latency: 10ms
  - Minimum frame rate: 60 FPS
  - Maximum memory usage: 512MB
  - Startup time: < 3 seconds

## 3. Integration with Existing Components

### 3.1 Domain Model Integration
- Connect UI components to existing domain models:
  - Music entities (Pitch, Duration, Interval, Voice, Score)
  - Rule validation system (ParallelFifthsRule, ParallelOctavesRule, VoiceLeadingRule)
  - Event systems (EventBus, EventStore, NoteEventHandler, NoteAddedEvent)
  - MelodicTemplate system for exercise generation

### 3.2 Infrastructure Integration
- Integrate existing adapters:
  - RtMidiAdapter for MIDI input/output (leveraging error handling and device management)
  - ScoreRepository implementations (InMemoryScoreRepository, CachingScoreRepository)
  - TemplateBasedExerciseGenerator for exercise management
  - EventSourcedRepository for history-based persistence

### 3.3 Error Handling Integration
- Connect to existing error handling system (using DomainErrors and ErrorHandler)
- Visualize errors from MidiError and other domain-specific errors
- Implement user-friendly error presentation with recovery options

## 4. Key UI Components

### 4.1 Main Window Layout

#### Score View
- Piano roll display with multiple voices
- Dynamic grid system based on ViewportSpec implementation
- Visualization of Voice and Note objects with proper pitch and duration rendering
- Integration with the existing Score model (including measure management)
- Support for smooth scrolling and zooming using the grid expansion mechanics

#### Control Panel
- Transport controls (Play, Stop, Record)
- Tempo control with spinner and slider (40-208 BPM)
- Metronome toggle
- MIDI device selection dropdown (using RtMidiAdapter's port enumeration)
- Voice mute/solo buttons with visual indicators for active voices

#### Exercise Panel
- Exercise description text area
- Rule violation indicators (connected to ValidationPipeline)
- Progress indicator with completion percentage
- Hint system with multiple levels of assistance based on current rules
- Solution checker with feedback display highlighting specific rule violations

#### Feedback Area
- Real-time rule validation displays (showing ParallelFifthsRule, ParallelOctavesRule, VoiceLeadingRule violations)
- Voice-specific feedback panels with interval analysis
- Historical mistakes analysis charts
- Improvement suggestions based on common errors

### 4.2 Secondary Windows

#### Exercise Browser
- List view with difficulty levels
- Category filter sidebar
- Progress tracking indicators
- Custom exercise creation interface (utilizing MelodicTemplate system)

#### Settings Dialog
- MIDI configuration panel (interfacing with RtMidiAdapter's ALSA/JACK support)
- Audio settings with device selection
- UI preferences (themes, colors, etc.)
- Rule set management interface for customizing validation rules

## 5. Viewport Implementation

### 5.1 Core Features
- Implementation based on ViewportSpec.md and ViewportSummary.md
- Default grid size: One octave (12 semitones) vertical, viewport width horizontal
- Dynamic expansion based on user interaction and note content
- Sparse matrix representation for optimal memory usage with large scores
- Double-buffered rendering for smooth updates during MIDI input

### 5.2 Expansion Mechanics
- Implement exact specifications from ViewportSpec.md:
  - Vertical expansion in 6 semitone increments when approaching viewport edges
  - Horizontal expansion by 50% of current viewport width when scrolling right
  - Memory optimization through unloading distant regions beyond buffer zones
 
### 5.3 Performance Optimizations
- Implement specific strategies from ViewportSummary.md:
  - Event batching for viewport updates to avoid redraws during rapid note input
  - Coordinate transformation caching for efficient musical-to-screen space conversion
  - Asynchronous content loading using Qt's threading facilities
  - Spatial indexing for efficient note lookup during playback and editing

### 5.4 Integration with Score and MIDI Systems
- Coordinate mapping between musical and screen spaces using ScoreViewportBridge
- Real-time input handling with RtMidiAdapter's event callback system
- Playback position tracking with auto-scroll during MIDI output
- Visual feedback for note input and playback with velocity sensitivity

## 6. Development Phases

### Phase 1: Core UI Framework and Domain Integration (Weeks 1-2)
- Setup Qt6 project structure with CMake integration
- Create initial window layout with docking panels
- Implement basic UI component classes (ScoreView, TransportControls, etc.)
- Establish integration with existing domain models (Score, Voice, Note)
- Connect to EventBus for domain event handling

### Phase 2: Viewport Implementation (Weeks 3-4)
- Implement NoteGrid class with GridDimensions following ViewportSpec
- Develop ViewportManager with proper state handling
- Create coordinate mapping system between musical and screen space
- Implement expansion triggers based on user interaction
- Test viewport performance with large scores

### Phase 3: MIDI Integration (Weeks 5-6)
- Connect UI to RtMidiAdapter's event system
- Implement device configuration interface with port enumeration
- Create real-time note input visualization with latency optimization
- Develop playback position tracking with auto-scroll
- Test cross-platform MIDI functionality (ALSA, JACK, Windows MM)

### Phase 4: Score Editing and Rule Visualization (Weeks 7-8)
- Implement note editing capabilities with undo/redo
- Create rule visualization system for ValidationPipeline results
- Develop interactive feedback system for rule violations
- Implement selection handling and manipulation tools
- Connect to validation system for real-time rule checking

### Phase 5: Exercise System Integration (Weeks 9-10)
- Connect to TemplateBasedExerciseGenerator
- Implement MelodicTemplate visualization
- Create exercise browser with difficulty progression
- Build progress tracking components with analytics
- Develop hint system with multiple assistance levels

### Phase 6: Optimization and Testing (Weeks 11-12)
- Performance profiling focused on MIDI latency
- Memory usage optimization for large scores
- Cross-platform testing and refinement
- User experience testing with musicians
- Documentation and final polish

## 7. Testing Strategy

### 7.1 Integration Testing
- Test integration with existing domain components
- Validate event propagation between UI and domain layers
- Verify rule visualization correctly reflects validation results
- Confirm MIDI input/output synchronization with RtMidiAdapter
- Test event handling with EventBus

### 7.2 Performance Testing
- Input latency measurements (target: <10ms)
- Frame rate monitoring during complex interactions
- Memory usage tracking during large score editing
- Load time analysis for different score sizes
- Viewport expansion performance under stress

### 7.3 User Experience Testing
- Navigation flow evaluation with actual musicians
- Control responsiveness assessment during playback
- Visual feedback effectiveness for rule violations
- Learning curve analysis for new users
- Cross-platform experience consistency

## 8. Implementation Guidelines

### 8.1 Code Organization
- Follow the existing project structure:
  - Add presentation/ directory with subdirectories for UI components
  - Implement ViewModels in a viewmodels/ subdirectory
  - Place Qt-specific adapters in adapters/ directory
  - Define UI interfaces in include/ directory

### 8.2 Performance Considerations
- Use Qt's graphics view framework for efficient rendering
- Implement lazy loading for UI components
- Optimize painting operations for score rendering
- Use event-driven updates instead of polling
- Leverage GPU acceleration where available

### 8.3 Cross-Platform Compatibility
- Use Qt's platform abstraction for core functionality
- Create platform-specific adaptations where needed (especially for MIDI)
- Test on all target platforms (Windows, Linux)
- Address platform-specific font rendering and DPI issues
- Create platform-specific installers with appropriate dependencies

## 9. Risks and Mitigations

### 9.1 Potential Risks
- Integration issues between UI layer and existing domain components
- MIDI latency exceeding requirements, especially on Linux with JACK
- Viewport performance issues with large scores and rapid input
- Memory usage spikes during complex operations
- Cross-platform inconsistencies in rendering and behavior

### 9.2 Mitigation Strategies
- Create comprehensive adapter layer between UI and domain
- Implement dedicated MIDI thread with high priority scheduling
- Use progressive loading and unloading for viewport with proper memory management
- Implement memory monitoring and optimization with caching strategies
- Establish platform-specific test suites to catch inconsistencies early

## 10. Success Criteria
- All UI components successfully integrated with existing domain models
- Performance metrics meeting or exceeding requirements (10ms latency, 60 FPS)
- Complete implementation of viewport system according to ViewportSpec
- Support for both ALSA and JACK MIDI on Linux platforms
- Successful user testing feedback from musicians