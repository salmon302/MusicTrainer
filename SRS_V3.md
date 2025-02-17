# Music Theory Training Tool - Software Requirements Specification V3

## 1. System Overview

### 1.1 Purpose
An advanced music theory training application specializing in counterpoint education through interactive MIDI-based exercises, with real-time feedback and progressive learning paths.

### 1.2 Core Features
1. Interactive counterpoint exercises
2. Real-time MIDI input/output
3. Progressive learning system
4. Detailed visual feedback
5. Customizable rule sets
6. Exercise history and analytics

## 2. User Interface Specifications

### 2.1 Main Window Layout
1. **Score View**
   - Piano roll display with multiple voices
   - Measure numbers and bar lines
   - Time signature display
   - Key signature indicator
   - Voice labels and colors
   - Note velocity visualization

2. **Control Panel**
   - Transport controls (Play, Stop, Record)
   - Tempo control (40-208 BPM)
   - Metronome toggle
   - MIDI device selection
   - Voice mute/solo buttons

3. **Exercise Panel**
   - Current exercise description
   - Rule violation indicators
   - Progress indicator
   - Hint system
   - Solution checker

4. **Feedback Area**
   - Real-time rule validation
   - Voice-specific feedback
   - Historical mistakes analysis
   - Improvement suggestions

### 2.2 Secondary Windows
1. **Exercise Browser**
   - Difficulty levels (Beginner to Advanced)
   - Category filters
   - Progress tracking
   - Custom exercise creation

2. **Settings Dialog**
   - MIDI configuration
   - Audio settings
   - UI preferences
   - Rule set management

## 3. Music Theory Components

### 3.1 Counterpoint Rules
1. **Melodic Rules**
   - Maximum interval size (P8)
   - Prohibited melodic intervals (tritone)
   - Step-wise motion preference
   - Leap compensation
   - Range limits per voice

2. **Harmonic Rules**
   - Parallel fifths/octaves prohibition
   - Contrary motion preference
   - Consonant interval requirements
   - Voice crossing prevention
   - Spacing requirements

3. **Rhythmic Rules**
   - Note duration restrictions
   - Syncopation handling
   - Rhythmic variety requirements
   - Cadence patterns

### 3.2 Exercise Types
1. **Species Counterpoint**
   - First species (1:1)
   - Second species (2:1)
   - Third species (4:1)
   - Fourth species (syncopated)
   - Fifth species (florid)

2. **Free Counterpoint**
   - Two-voice exercises
   - Three-voice exercises
   - Four-voice exercises
   - Bass line exercises
   - Melody harmonization

## 4. MIDI Integration

### 4.1 Input Handling
1. **Real-time Processing**
   - Note-on/off events
   - Velocity sensitivity
   - Sustain pedal support
   - Multi-channel input

2. **Recording Modes**
   - Voice-specific recording
   - Step recording
   - Loop recording
   - Punch-in/out

### 4.2 Output Generation
1. **Playback Features**
   - Voice-specific playback
   - Metronome click
   - Count-in measures
   - Tempo ramping

2. **MIDI Routing**
   - Multiple device support
   - Channel mapping
   - Program changes
   - Control changes

## 5. Learning System

### 5.1 Progression Paths
1. **Beginner Path**
   - Basic intervals
   - Simple melodies
   - First species only
   - Limited voice range

2. **Intermediate Path**
   - Complex intervals
   - All species counterpoint
   - Extended voice range
   - Modal exercises

3. **Advanced Path**
   - Free counterpoint
   - Multiple voices
   - Complex rhythms
   - Style-specific exercises

### 5.2 Feedback System
1. **Real-time Feedback**
   - Visual rule violations
   - Suggested corrections
   - Voice-specific hints
   - Performance metrics

2. **Historical Analysis**
   - Common mistake patterns
   - Progress tracking
   - Skill assessment
   - Practice recommendations

## 6. Technical Requirements

### 6.1 Performance
- Maximum latency: 10ms
- Minimum frame rate: 60 FPS
- Maximum memory usage: 512MB
- Startup time: < 3 seconds

### 6.2 Compatibility
- Windows 10/11
- MIDI 1.0/2.0 support
- Standard MIDI files
- VST3 host support

## 7. Data Management

### 7.1 User Data
- Exercise progress
- Custom exercises
- Performance history
- Settings profiles

### 7.2 Exercise Data
- Built-in exercise library
- User-created exercises
- Difficulty ratings
- Solution examples

## 8. Future Considerations

### 8.1 Advanced Features
- AI-powered exercise generation
- Real-time collaboration
- Cloud sync
- Mobile companion app

### 8.2 Integration Options
- DAW plugin version
- Web-based version
- Educational institution features
- Professional certification tracking

#(Should be implemented with Qt6)
