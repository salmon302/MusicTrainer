# Software Requirements Specification - MusicTrainer V4

**Version:** 4.0
**Date:** 2025-04-08

## Table of Contents
1.  [Introduction](#1-introduction)
    1.1. [Purpose](#11-purpose)
    1.2. [Scope](#12-scope)
    1.3. [Definitions, Acronyms, and Abbreviations](#13-definitions-acronyms-and-abbreviations)
    1.4. [References](#14-references)
    1.5. [Overview](#15-overview)
2.  [Overall Description](#2-overall-description)
    2.1. [Product Perspective](#21-product-perspective)
    2.2. [Product Functions Summary](#22-product-functions-summary)
    2.3. [User Characteristics](#23-user-characteristics)
    2.4. [Constraints](#24-constraints)
    2.5. [Assumptions and Dependencies](#25-assumptions-and-dependencies)
3.  [Specific Requirements](#3-specific-requirements)
    3.1. [Functional Requirements](#31-functional-requirements)
    3.2. [Non-Functional Requirements](#32-non-functional-requirements)
    3.3. [Interface Requirements](#33-interface-requirements)
4.  [Data Management Requirements](#4-data-management-requirements)
5.  [Prioritization](#5-prioritization)
6.  [Future Considerations / Removed Features](#6-future-considerations--removed-features)
7.  [Appendices](#7-appendices)

---

## 1. Introduction

### 1.1. Purpose

This document specifies the software requirements for Version 4 (V4) of the MusicTrainer application. MusicTrainer is an advanced music theory training application specializing in counterpoint education. It aims to provide users with interactive MIDI-based exercises, real-time feedback, and progressive learning paths to improve their music theory skills, particularly in counterpoint composition. This SRS serves as the primary reference for the redesign and future development efforts, consolidating information from previous versions and analyses.

### 1.2. Scope

This SRS covers the functional and non-functional requirements for the redesigned MusicTrainer application (V4). The scope includes:
*   Core counterpoint exercise functionality (Species and Free).
*   Real-time MIDI input and output processing.
*   A progressive learning system with defined paths.
*   Detailed visual feedback mechanisms, including score view and rule violation indicators.
*   A modular and customizable rule set system with presets.
*   Exercise history, analytics, and user progress tracking.
*   A solution generation and hint system.
*   GUI-based note manipulation and score editing.
*   Voice analysis and visualization capabilities.
*   Performance, usability, and technical constraints.

Features explicitly marked as "Future Considerations" or identified as out of scope during analysis are not covered by this version. This document supersedes `SRS_V3.md`.

### 1.3. Definitions, Acronyms, and Abbreviations

*   **SRS:** Software Requirements Specification
*   **MIDI:** Musical Instrument Digital Interface
*   **GUI:** Graphical User Interface
*   **FPS:** Frames Per Second
*   **VST3:** Virtual Studio Technology 3 (Audio Plugin Standard)
*   **DAW:** Digital Audio Workstation
*   **Cantus Firmus (CF):** A pre-existing melody forming the basis of a polyphonic composition.
*   **Species Counterpoint:** A pedagogical method for learning counterpoint through stages of increasing complexity.
*   **Free Counterpoint:** Counterpoint writing with fewer restrictions than species counterpoint.
*   **Preset:** A pre-configured set of rules and parameters for an exercise or validation context.
*   **Domain Model:** The representation of musical concepts (notes, scores, rules) within the software.
*   **Rule Engine:** The component responsible for validating scores against musical rules.
*   **Viewport:** The visible area of the score displayed in the GUI.

### 1.4. References

*   `SRS_V3.md`: Previous Software Requirements Specification.
*   `RequirementsAnalysis.md`: Analysis of gaps and required additions based on V3.
*   `UserTesting.md`: User testing plan, results, and derived priorities.
*   `TechnicalSpecification.md`: Details on architecture, algorithms, and data structures.
*   `memory-bank/productContext.md`: High-level project goals and features.
*   `Species counterpoint.md`: Detailed explanation of species counterpoint rules.
*   `README.md`: Project overview and setup instructions.
*   `ArchitectureDecisions.md`: Log of architectural choices.
*   `StateManagement.md`: Details on application state handling.
*   IEEE Std 830-1998: Recommended Practice for Software Requirements Specifications (Reference for structure inspiration).

### 1.5. Overview

This document is organized into the following sections:
*   **Section 1 (Introduction):** Provides purpose, scope, definitions, references, and an overview of the SRS.
*   **Section 2 (Overall Description):** Describes the product perspective, summarizes functions, defines user characteristics, and lists constraints and assumptions.
*   **Section 3 (Specific Requirements):** Details the functional, non-functional, and interface requirements.
*   **Section 4 (Data Management):** Outlines requirements for handling user, exercise, and preset data.
*   **Section 5 (Prioritization):** Defines the priority levels for implementing requirements based on analysis and testing.
*   **Section 6 (Future Considerations / Removed Features):** Lists features deferred to future versions or explicitly removed.
*   **Section 7 (Appendices):** Contains supplementary information.

This SRS aims to provide a clear, concise, and comprehensive specification for the development team.

---

## 2. Overall Description

### 2.1. Product Perspective

MusicTrainer V4 is intended as a significant redesign and enhancement of the previous version (V3). It operates as a standalone desktop application for Windows and Linux (macOS support TBD), built using C++ and the Qt6 framework. It interfaces with external MIDI hardware for input and output. The application provides a specialized educational environment focused on counterpoint, distinct from general-purpose Digital Audio Workstations (DAWs), although future integration (e.g., VST3 plugin) is a consideration. It builds upon the core concepts of V3 but addresses identified shortcomings in usability, feature completeness, and technical implementation as detailed in `RequirementsAnalysis.md` and `UserTesting.md`.

### 2.2. Product Functions Summary

The primary functions of MusicTrainer V4 include:
*   **Score Creation & Editing:** Allowing users to input and manipulate musical notes via mouse and MIDI keyboard within a piano-roll style viewport.
*   **Counterpoint Exercise Interaction:** Providing structured exercises (Species and Free Counterpoint) based on a Cantus Firmus or other templates.
*   **Real-time Rule Validation:** Continuously checking the user's input against selected musical rule sets (melodic, harmonic, rhythmic) and providing immediate visual feedback.
*   **MIDI Input/Output:** Capturing musical performance data from MIDI controllers and playing back scores via MIDI devices with low latency.
*   **Learning Progression:** Offering structured learning paths (Beginner, Intermediate, Advanced) with increasing complexity.
*   **Feedback & Analysis:** Displaying detailed information about rule violations, voice leading, intervals, and providing suggestions for improvement.
*   **Solution & Hint Generation:** Providing users with hints or complete solutions for exercises based on the active rule set.
*   **Rule Set & Preset Management:** Allowing users to select, customize, and save different sets of counterpoint rules (presets).
*   **User Progress Tracking:** Recording exercise history and performance analytics (details TBD).

### 2.3. User Characteristics

The target users for MusicTrainer V4 include:
*   **Beginner Music Students:** Users with basic music notation knowledge seeking foundational understanding and practice in counterpoint. Require clear guidance, simple interfaces, and progressive difficulty.
*   **Intermediate Musicians:** Users with some composition or counterpoint experience aiming to refine their skills. Expect more complex exercises and detailed feedback.
*   **Advanced Musicians/Educators:** Composers, arrangers, or teachers using the tool for advanced practice, analysis, or demonstrating concepts. May require highly customizable rules and analysis tools.
*   **Technical Users:** (Primarily for testing/evaluation) Users capable of assessing MIDI performance, latency, and system integration.

Usability findings from `UserTesting.md` indicate a need for clearer UI element purpose, intuitive navigation (especially viewport controls like zoom/pan), and robust input methods.

### 2.4. Constraints

*   **Platform:** Windows 10/11, Linux (Ubuntu/Debian recommended). macOS support is desirable but secondary.
*   **Framework:** C++17, Qt 6 framework.
*   **Performance:**
    *   Maximum MIDI input-to-output latency: <= 10ms (Target from `SRS_V3.md` and `UserTesting.md`).
    *   Minimum UI frame rate: 60 FPS (Target from `SRS_V3.md` and `UserTesting.md`).
    *   Maximum memory usage: 512MB (Target from `SRS_V3.md`, may need re-evaluation).
    *   Rule validation response time: < 50ms (Target from `UserTesting.md`).
*   **Architecture:** Must adhere to the layered, event-driven architecture outlined in `TechnicalSpecification.md` and `productContext.md` (e.g., Domain Model, Rule Engine, Viewport Integration). Use of immutable domain objects, plugin architecture for rules.
*   **MIDI:** Must support standard MIDI 1.0 devices. MIDI 2.0 support is a future consideration.
*   **Dependencies:** Core dependencies include CMake, C++17 compiler, ALSA (Linux). Qt6 is required for the GUI.

### 2.5. Assumptions and Dependencies

*   **User Hardware:** Users are assumed to have a computer meeting minimum system requirements (to be defined) and access to standard MIDI controllers for full functionality.
*   **MIDI Environment:** Assumes a stable MIDI environment on the user's operating system with correctly installed drivers.
*   **Music Theory Knowledge:** Users are assumed to have a basic understanding of music notation. The application teaches counterpoint, not basic notation.
*   **Development Environment:** Build process depends on CMake and standard C++ toolchains.
*   **External Libraries:** Relies on Qt6 for the GUI and potentially other libraries for specific features (e.g., MIDI handling - RtMidi mentioned in `UserTesting.md`).

---

## 3. Specific Requirements

### 3.1. Functional Requirements

This section details the functional requirements of MusicTrainer V4, describing what the system must do.

#### 3.1.1. Score View / Viewport

The Score View is the primary interface for displaying and interacting with the musical score. It shall function as an interactive piano roll.

| Req ID  | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                 |
| :------ | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :-------------------------------------------------------------------- |
| **FV-01** | The Score View shall display musical notes on a grid representing pitch (vertical axis) and time (horizontal axis).    | `SRS_V3.md` 2.1.1                                                        | High     | Baseline    | Piano roll style.                                                     |
| **FV-02** | The Score View shall display multiple independent voices simultaneously, distinguished by color.                       | `SRS_V3.md` 2.1.1, 12.1                                                  | High     | Baseline    | Voice colors should be configurable (See UI Preferences).             |
| **FV-03** | The Score View shall display measure numbers and bar lines according to the current time signature.                    | `SRS_V3.md` 2.1.1, 12.1                                                  | High     | Baseline    |                                                                       |
| **FV-04** | The Score View shall display the active time signature.                                                                | `SRS_V3.md` 2.1.1, 12.1                                                  | High     | Baseline    |                                                                       |
| **FV-05** | The Score View shall display the active key signature.                                                                 | `SRS_V3.md` 2.1.1, 12.1                                                  | High     | Baseline    |                                                                       |
| **FV-06** | The Score View shall display labels for each voice.                                                                    | `SRS_V3.md` 2.1.1, 12.1                                                  | Medium   | Baseline    |                                                                       |
| **FV-07** | The Score View shall visualize note velocity (e.g., via color intensity or opacity).                                   | `SRS_V3.md` 2.1.1, 12.1                                                  | Medium   | Baseline    | Method TBD.                                                           |
| **FV-08** | The Score View shall support vertical scrolling to navigate pitches.                                                   | `UserTesting.md` 5.1.2                                                 | High     | Baseline    | Smooth scrolling required.                                            |
| **FV-09** | The Score View shall support horizontal scrolling to navigate time.                                                    | `UserTesting.md` 5.1.2                                                 | High     | Baseline    | Smooth scrolling required.                                            |
| **FV-10** | The Score View shall support zooming in and out on both pitch and time axes.                                           | `UserTesting.md` 5.1.2                                                 | High     | Fix         | Must be functional. Standard controls (e.g., Ctrl+MouseWheel) expected. |
| **FV-11** | The Score View shall provide a mechanism for quickly jumping to specific measures or sections (e.g., minimap, markers). | `UserTesting.md` 5.1.2                                                 | Medium   | Enhancement | Addresses navigation gap identified in testing.                       |
| **FV-20** | The Score View shall allow independent scaling/zooming of the horizontal (time) and vertical (pitch) axes.             | User Feedback                                                          | Medium   | Enhancement | Provides finer control over grid appearance, similar to DAWs.         |
| **FV-21** | The Score View shall provide intuitive controls for adjusting horizontal and vertical zoom levels (e.g., dragging axes, modifier keys + scroll). | User Feedback, Derived from FV-20                                      | Medium   | Enhancement | Specific interaction methods TBD during UI design.                    |
| **FV-22** | The system shall allow the user to optionally define a fixed visible vertical pitch range (e.g., set min/max octave or note). | User Feedback                                                          | Medium   | Enhancement | Allows users to focus the view, hiding unused pitch ranges.           |
| **FV-23** | When a fixed vertical pitch range is set (FV-22), scrolling shall be constrained within that range, and the vertical zoom shall adapt to fit the range within the view height. | Derived from FV-22                                                     | Medium   | Enhancement | Defines behavior when fixed range is active.                          |
| **FV-12** | The Score View grid shall expand dynamically horizontally as notes are added or playback progresses beyond current bounds. | `UserTesting.md` 12.1, `TechnicalSpecification.md` 1.3, 3.4, 4.3       | High     | Baseline    | Requires efficient grid expansion management (see NF requirements).   |
| **FV-13** | The Score View grid shall expand dynamically vertically if notes are added outside the current pitch range.            | `UserTesting.md` 12.1, `TechnicalSpecification.md` 1.3, 3.4, 4.3       | High     | Baseline    | Requires efficient grid expansion management.                         |
| **FV-14** | The Score View shall provide visual feedback when hovering over valid note placement locations.                        | `UserTesting.md` 5.2.1                                                 | High     | Fix         | Addresses usability issue from testing.                               |
| **FV-15** | The Score View shall support selection of single notes via mouse click.                                                | `UserTesting.md` 5.2.3                                                 | High     | Fix         | Prerequisite for note editing.                                        |
| **FV-16** | The Score View shall support selection of multiple notes (e.g., via rectangle drag, Shift+click).                      | `UserTesting.md` 5.2.3                                                 | High     | Enhancement | Prerequisite for bulk editing.                                        |
| **FV-17** | The Score View shall clearly indicate selected notes.                                                                  | `UserTesting.md` 5.2.3                                                 | High     | Fix         |                                                                       |
| **FV-18** | The Score View shall maintain smooth performance during navigation (scrolling, zooming) even with large scores.        | `UserTesting.md` 6.2, `TechnicalSpecification.md` 3.4                  | High     | Baseline    | See NF-PERF requirements.                                             |
| **FV-19** | The Score View shall center the view appropriately when loading a new exercise or score.                               | `UserTesting.md` 13.1.1                                                | Medium   | Fix         | Addresses usability issue from testing.                               |

#### 3.1.2. Note Input and Editing

These requirements cover the creation, modification, and deletion of notes within the Score View.

| Req ID  | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                     |
| :------ | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :------------------------------------------------------------------------ |
| **FN-01** | The system shall allow users to add notes to a selected voice by clicking at the desired pitch and time position in the Score View. | `UserTesting.md` 5.2.1                                                 | High     | Fix         | Currently non-functional. Requires fixing `ScoreView::mouseReleaseEvent`. |
| **FN-02** | The system shall allow users to add notes to a selected voice using a connected MIDI input device (keyboard).          | `SRS_V3.md` 4.1, `UserTesting.md` 5.2.2                                | High     | Fix         | Requires MIDI input integration (`RtMidiAdapter`, `MIDIEventSystem`).     |
| **FN-03** | Notes added via MIDI shall capture pitch, velocity, and timing accurately (respecting quantization settings - TBD).    | `SRS_V3.md` 4.1.1, `UserTesting.md` 5.2.2                              | High     | Fix         | Quantization details need further definition.                             |
| **FN-04** | The system shall allow users to delete selected notes (single or multiple).                                            | `UserTesting.md` 5.2.3, `RequirementsAnalysis.md` 4                    | High     | Fix         | Standard keyboard shortcuts (e.g., Delete key) should be supported.       |
| **FN-05** | The system shall allow users to change the pitch of selected notes by dragging them vertically in the Score View.      | `UserTesting.md` 5.2.3, `RequirementsAnalysis.md` 4                    | High     | Fix         | Requires implementing selection and drag-and-drop.                        |
| **FN-06** | The system shall allow users to change the start time/position of selected notes by dragging them horizontally.        | `UserTesting.md` 5.2.3, `RequirementsAnalysis.md` 4                    | High     | Fix         | Requires implementing selection and drag-and-drop.                        |
| **FN-07** | The system shall allow users to change the duration of selected notes (e.g., by dragging the note's end handle).       | `UserTesting.md` 5.2.3                                                 | High     | Fix         | Requires note duration handle implementation.                             |
| **FN-08** | The system shall provide a mechanism to edit note properties (e.g., exact velocity, duration) via a dedicated editor or panel. | `RequirementsAnalysis.md` 4                                            | Medium   | Enhancement | Addresses gap identified in analysis.                                     |
| **FN-09** | The system shall support step recording mode via MIDI input.                                                           | `SRS_V3.md` 4.1.2                                                        | Medium   | Baseline    | Specifics of step recording UI/UX TBD.                                    |
| **FN-10** | The system shall provide undo/redo functionality for note input and editing operations.                                | `RequirementsAnalysis.md` 4                                            | High     | Enhancement | Addresses gap identified in analysis. Requires Command pattern (`NoteCommand`). |

#### 3.1.3. MIDI Integration

These requirements cover the interaction with external MIDI devices for input and output.

| Req ID   | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :--------------------------------------------------------------------------------------------------- |
| **FM-01**  | The system shall detect and allow users to select available MIDI input devices.                                        | `SRS_V3.md` 2.1.2, 2.2.2, `UserTesting.md` 6.1                         | High     | Baseline    | Requires robust device detection and handling.                                                       |
| **FM-02**  | The system shall detect and allow users to select available MIDI output devices.                                       | `SRS_V3.md` 2.1.2, 2.2.2, 4.2.1, `UserTesting.md` 6.1                  | High     | Baseline    |                                                                                                      |
| **FM-03**  | The system shall process incoming MIDI Note On/Off events in real-time for note input (See FN-02).                     | `SRS_V3.md` 4.1.1                                                        | High     | Fix         | Requires low latency processing (See NF-PERF-01). Lock-free queue mentioned in `TechnicalSpecification.md`. |
| **FM-04**  | The system shall process incoming MIDI velocity data for note input (See FN-03).                                       | `SRS_V3.md` 4.1.1                                                        | High     | Fix         |                                                                                                      |
| **FM-05**  | The system shall process incoming MIDI sustain pedal (CC64) events.                                                    | `SRS_V3.md` 4.1.1                                                        | Medium   | Baseline    | Effect on playback/recording TBD.                                                                    |
| **FM-06**  | The system shall support receiving MIDI input on multiple channels simultaneously (if device supports it).             | `SRS_V3.md` 4.1.1                                                        | Medium   | Baseline    | Primarily for multi-voice input scenarios.                                                           |
| **FM-07**  | The system shall play back the score content via the selected MIDI output device.                                      | `SRS_V3.md` 4.2.1, `UserTesting.md` 5.1.3                              | High     | Fix         | Requires connection to transport controls.                                                           |
| **FM-08**  | Playback shall support individual voice muting and soloing.                                                            | `SRS_V3.md` 2.1.2                                                        | High     | Baseline    | Requires UI controls and corresponding playback logic.                                               |
| **FM-09**  | Playback shall include an optional metronome click sent to the MIDI output.                                            | `SRS_V3.md` 2.1.2, 4.2.1                                                 | High     | Fix         | Requires connection to metronome toggle UI and audio/MIDI generation.                                |
| **FM-10** | Playback shall support starting/stopping via transport controls (See UI Requirements).                                 | `SRS_V3.md` 2.1.2, `UserTesting.md` 5.1.3                              | High     | Fix         |                                                                                                      |
| **FM-11** | The system shall allow mapping specific MIDI input channels to specific voices in the score.                           | `SRS_V3.md` 4.2.2                                                        | Medium   | Enhancement | Useful for multi-channel controllers or recording multiple parts.                                    |
| **FM-12** | The system shall handle MIDI device connection/disconnection gracefully (e.g., automatic reconnection attempts, user notification). | `TechnicalSpecification.md` 6.2                                        | Medium   | Enhancement | Improves robustness.                                                                                 |
| **FM-13** | MIDI processing (input to output loop) shall meet low latency requirements (See NF-PERF-01).                           | `SRS_V3.md` 6.1, `UserTesting.md` 5.2.2, 6.1, 9.2                      | High     | Baseline    | Critical for usability.                                                                              |

#### 3.1.4. Rule Engine and Validation

These requirements define the system responsible for checking the score against defined musical rules.

| Req ID   | Requirement Description                                                                                                | Source                                                                                   | Priority | Status      | Notes                                                                                                                               |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **FR-01**  | The system shall provide a rule engine capable of validating a score against a configurable set of musical rules.        | `SRS_V3.md` 3.1, `TechnicalSpecification.md` 1.1                                         | High     | Baseline    | Core functionality.                                                                                                                 |
| **FR-02**  | The rule engine shall perform validation in real-time or near real-time as the user modifies the score.                | `SRS_V3.md` 2.1.4, `UserTesting.md` 5.3.2                                                | High     | Baseline    | Performance constraints apply (See NF-PERF-04). Incremental validation preferred (`TechnicalSpecification.md` 3.1). |
| **FR-03**  | The system shall provide immediate visual feedback in the Score View indicating notes or sections violating active rules. | `SRS_V3.md` 2.1.4, 5.2.1, `UserTesting.md` 5.3.2, 13.1.2                                 | High     | Fix         | Requires clear visual indicators (e.g., highlighting, icons).                                                       |
| **FR-04**  | The system shall provide detailed textual feedback explaining rule violations in a dedicated Feedback Area.            | `SRS_V3.md` 2.1.4, 5.2.1, `UserTesting.md` 5.4.1, 12.2                                   | High     | Fix         | Feedback should be clear, specific, and reference the rule violated.                                                |
| **FR-05**  | The rule engine shall support defining and validating Melodic Rules.                                                   | `SRS_V3.md` 3.1.1                                                                        | High     | Baseline    |                                                                                                                     |
| **FR-05a** | Melodic Rule Example: Prohibit configurable melodic intervals (e.g., augmented seconds, tritones).                     | `SRS_V3.md` 3.1.1, `Species counterpoint.md`                                             | High     | Baseline    | Specific intervals should be configurable per preset.                                                               |
| **FR-05b** | Melodic Rule Example: Enforce maximum melodic leap size (e.g., P8).                                                    | `SRS_V3.md` 3.1.1, `Species counterpoint.md`                                             | High     | Baseline    | Size should be configurable.                                                                                        |
| **FR-05c** | Melodic Rule Example: Enforce leap compensation (e.g., step in opposite direction after leap).                         | `SRS_V3.md` 3.1.1, `Species counterpoint.md`                                             | High     | Baseline    | Specifics may vary by style/preset.                                                                                 |
| **FR-05d** | Melodic Rule Example: Enforce voice range limits.                                                                      | `SRS_V3.md` 3.1.1, `Species counterpoint.md`                                             | High     | Baseline    | Range limits should be configurable per voice/preset.                                                               |
| **FR-06**  | The rule engine shall support defining and validating Harmonic Rules (intervals between simultaneous notes).           | `SRS_V3.md` 3.1.2                                                                        | High     | Baseline    |                                                                                                                     |
| **FR-06a** | Harmonic Rule Example: Prohibit parallel perfect fifths and octaves between voices.                                    | `SRS_V3.md` 3.1.2, `Species counterpoint.md`                                             | High     | Baseline    | Core counterpoint rule.                                                                                             |
| **FR-06b** | Harmonic Rule Example: Prohibit direct (hidden) perfect fifths and octaves in specific contexts.                       | `Species counterpoint.md`                                                                | High     | Baseline    | Contexts (e.g., outer voices, similar motion) should be configurable.                                               |
| **FR-06c** | Harmonic Rule Example: Enforce requirements for consonant intervals on specific beats (depending on species/style).    | `SRS_V3.md` 3.1.2, `Species counterpoint.md`                                             | High     | Baseline    | Consonance definitions may vary.                                                                                    |
| **FR-06d** | Harmonic Rule Example: Limit or prohibit voice crossing.                                                               | `SRS_V3.md` 3.1.2, `Species counterpoint.md`                                             | High     | Baseline    | Configurable per preset.                                                                                            |
| **FR-06e** | Harmonic Rule Example: Enforce maximum/minimum spacing between voices.                                                 | `SRS_V3.md` 3.1.2, `Species counterpoint.md`                                             | High     | Baseline    | Spacing limits configurable.                                                                                        |
| **FR-07**  | The rule engine shall support defining and validating Rhythmic Rules.                                                  | `SRS_V3.md` 3.1.3                                                                        | High     | Baseline    |                                                                                                                     |
| **FR-07a** | Rhythmic Rule Example: Enforce note duration restrictions (e.g., in specific species).                                 | `SRS_V3.md` 3.1.3, `Species counterpoint.md`                                             | High     | Baseline    | Applicable primarily to species counterpoint exercises.                                                             |
| **FR-07b** | Rhythmic Rule Example: Validate handling of syncopation (e.g., preparation/resolution in 4th species).                 | `SRS_V3.md` 3.1.3, `Species counterpoint.md`                                             | High     | Baseline    |                                                                                                                     |
| **FR-07c** | Rhythmic Rule Example: Validate cadence patterns.                                                                      | `SRS_V3.md` 3.1.3, `Species counterpoint.md`                                             | Medium   | Baseline    | Specific patterns TBD.                                                                                              |
| **FR-08**  | The rule engine shall support enabling/disabling individual rules.                                                     | Implied by `SRS_V3.md` 1.2.5, `RequirementsAnalysis.md` 1                                | High     | Baseline    | Essential for customization and presets.                                                                            |
| **FR-09**  | The rule engine shall allow configuration of rule parameters (e.g., max leap size, allowed intervals).                 | Implied by `SRS_V3.md` 1.2.5, `RequirementsAnalysis.md` 1                                | High     | Enhancement | Essential for customization and presets.                                                                            |
| **FR-10** | The rule engine architecture shall be extensible to allow adding new rule types via a plugin system.                   | `productContext.md` 26, `TechnicalSpecification.md` 2.10                                 | Medium   | Baseline    | Architectural requirement.                                                                                          |
| **FR-11** | The rule engine shall be capable of detecting potential conflicts between enabled rules (e.g., contradictory constraints). | `RequirementsAnalysis.md` 1.1, `TechnicalSpecification.md` 1.1, 4.2                    | Medium   | Enhancement | Important for preset validation.                                                                                    |
| **FR-12** | Rule validation results shall be used by the Solution/Hint generation system (See Section 3.1.x).                      | `RequirementsAnalysis.md` 2, `TechnicalSpecification.md` 1.2                             | High     | Baseline    | System interdependency.                                                                                             |

#### 3.1.5. Exercises and Learning System

These requirements define the exercises provided to the user and the system for managing learning progression.

| Req ID   | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                             |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :------------------------------------------------------------------------------------------------ |
| **FE-01**  | The system shall provide interactive counterpoint exercises based on a Cantus Firmus (CF) or other templates.          | `SRS_V3.md` 1.2.1, 3.2                                                 | High     | Baseline    | Core exercise format.                                                                             |
| **FE-02**  | The system shall provide exercises covering the five species of counterpoint.                                          | `SRS_V3.md` 3.2.1                                                        | High     | Baseline    | First (1:1), Second (2:1), Third (4:1), Fourth (syncopated), Fifth (florid).                      |
| **FE-03**  | The system shall provide exercises covering free counterpoint.                                                         | `SRS_V3.md` 3.2.2                                                        | High     | Baseline    | Including 2, 3, and 4-voice exercises.                                                            |
| **FE-04**  | The system shall provide exercises for melody harmonization and bass line realization.                                 | `SRS_V3.md` 3.2.2                                                        | Medium   | Baseline    |                                                                                                   |
| **FE-05**  | The system shall provide an Exercise Browser for users to select exercises.                                            | `SRS_V3.md` 2.2.1, `UserTesting.md` 5.3.1                              | High     | Fix         | Currently non-functional. Needs implementation.                                                   |
| **FE-06**  | The Exercise Browser shall allow filtering exercises by difficulty level (e.g., Beginner, Intermediate, Advanced).     | `SRS_V3.md` 2.2.1, 5.1                                                 | High     | Fix         | Requires implementation.                                                                          |
| **FE-07**  | The Exercise Browser shall allow filtering exercises by category (e.g., Species 1, Free 2-Voice, Cadences).            | `SRS_V3.md` 2.2.1                                                        | High     | Fix         | Requires implementation. Categories TBD.                                                          |
| **FE-08**  | The system shall display the description and goals of the currently loaded exercise.                                   | `SRS_V3.md` 2.1.3                                                        | High     | Fix         | Requires Exercise Panel implementation.                                                           |
| **FE-09**  | The system shall track user progress through exercises and learning paths.                                             | `SRS_V3.md` 1.2.6, 2.2.1, 5.2.2, `RequirementsAnalysis.md` 3           | Medium   | Enhancement | Specific metrics and display TBD. Links to potential "Game Mode".                                 |
| **FE-10** | The system shall provide defined learning paths (e.g., Beginner, Intermediate, Advanced) guiding users through exercises. | `SRS_V3.md` 1.2.3, 5.1, `productContext.md` 16                         | High     | Baseline    | Content of paths needs definition.                                                                |
| **FE-11** | Beginner path exercises shall focus on basic intervals, simple melodies, first species, and limited ranges.            | `SRS_V3.md` 5.1.1                                                        | High     | Baseline    |                                                                                                   |
| **FE-12** | Intermediate path exercises shall introduce all species, complex intervals, extended ranges, and potentially modes.    | `SRS_V3.md` 5.1.2                                                        | High     | Baseline    |                                                                                                   |
| **FE-13** | Advanced path exercises shall focus on free counterpoint, multiple voices, complex rhythms, and potentially styles.    | `SRS_V3.md` 5.1.3                                                        | High     | Baseline    |                                                                                                   |
| **FE-14** | The system shall allow for the creation and loading of custom exercises (e.g., user-defined CF, specific rule sets).   | `SRS_V3.md` 2.2.1, 7.2.2                                                 | Medium   | Enhancement | File format and creation UI TBD.                                                                  |
| **FE-15** | The system shall include a library of built-in exercises covering different paths and categories.                      | `SRS_V3.md` 7.2.1, `UserTesting.md` 5.3.1                              | High     | Fix         | Requires creation of sample/default exercises.                                                    |
| **FE-16** | The system should provide mechanisms for scoring or evaluating exercise completion (potentially as part of a "Game Mode"). | `RequirementsAnalysis.md` 3                                            | Medium   | Enhancement | Addresses gap for testing/gamification. Scoring criteria TBD.                                     |
| **FE-17** | The system shall provide functionality to generate basic exercise templates (e.g., CF generation).                     | `RequirementsAnalysis.md` 5, `UserTesting.md` 5.3.1                    | Medium   | Enhancement | Links to Automatic Melody Generation requirement. `TemplateBasedExerciseGenerator` mentioned. |

#### 3.1.6. Preset System

These requirements define the system for managing, saving, loading, and applying sets of rules and configurations (Presets).

| Req ID   | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :--------------------------------------------------------------------------------------------------- |
| **FP-01**  | The system shall allow users to select from a list of predefined presets (e.g., "Strict Fuxian Species 1", "Basic Free Counterpoint"). | `RequirementsAnalysis.md` 1                                            | High     | Enhancement | Addresses gap in V3. Predefined presets need content definition.                                     |
| **FP-02**  | Applying a preset shall configure the active rule set (enabled rules and their parameters) in the Rule Engine.         | `RequirementsAnalysis.md` 1                                            | High     | Enhancement | Core function of presets.                                                                            |
| **FP-03**  | The system shall allow users to create custom presets by modifying existing ones or starting from scratch.             | `RequirementsAnalysis.md` 1                                            | High     | Enhancement | Requires UI for rule selection and parameter configuration.                                          |
| **FP-04**  | The system shall allow users to save their custom presets for later use.                                               | `RequirementsAnalysis.md` 1                                            | High     | Enhancement | Requires persistence mechanism. JSON serialization mentioned in `TechnicalSpecification.md`.         |
| **FP-05**  | The system shall allow users to load their saved custom presets.                                                       | `RequirementsAnalysis.md` 1                                            | High     | Enhancement |                                                                                                      |
| **FP-06**  | The system shall allow users to name and rename their custom presets.                                                  | `RequirementsAnalysis.md` 1                                            | High     | Enhancement | Basic preset management.                                                                             |
| **FP-07**  | The system shall allow users to delete their custom presets.                                                           | `RequirementsAnalysis.md` 1                                            | Medium   | Enhancement | Basic preset management.                                                                             |
| **FP-08**  | The system shall provide a mechanism to manage presets (e.g., in the Settings Dialog).                                 | `SRS_V3.md` 2.2.2 (Rule set management), `RequirementsAnalysis.md` 1   | High     | Enhancement | UI location TBD (Settings likely).                                                                   |
| **FP-09**  | The system should validate presets to check for rule incompatibilities or conflicts before saving/applying.            | `RequirementsAnalysis.md` 1, `TechnicalSpecification.md` 1.1, 2.1      | Medium   | Enhancement | Requires rule dependency/conflict detection logic (FR-11).                                           |
| **FP-10** | Preset configurations shall be persistent across application sessions.                                                 | `RequirementsAnalysis.md` 1                                            | High     | Enhancement | Saved presets must be available after restarting.                                                    |
| **FP-11** | The system should support hot-reloading of preset configurations (if feasible).                                        | `TechnicalSpecification.md` 2.1                                        | Low      | Enhancement | Allows updating presets without restarting (potentially for development/advanced users).             |

#### 3.1.7. Feedback and Analysis

These requirements define how the system provides feedback on the user's work and analyzes the musical content.

| Req ID   | Requirement Description                                                                                                | Source                                                                                   | Priority | Status      | Notes                                                                                                                            |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------- | :------- | :---------- | :------------------------------------------------------------------------------------------------------------------------------- |
| **FF-01**  | The system shall provide a dedicated Feedback Area in the UI to display validation results and analysis.               | `SRS_V3.md` 2.1.4, `UserTesting.md` 5.1.1                                                | High     | Fix         | Purpose needs to be clear to the user (identified in testing). Requires implementation.                                          |
| **FF-02**  | The Feedback Area shall display a list of current rule violations detected by the Rule Engine (FR-04).                 | `SRS_V3.md` 2.1.4, 5.2.1, `UserTesting.md` 5.4.1, 12.2                                   | High     | Fix         | Requires connection to Rule Engine.                                                                                              |
| **FF-03**  | Violation feedback shall clearly state the rule that was broken and the location (e.g., measure, beat, voice).         | `SRS_V3.md` 5.2.1, `UserTesting.md` 5.4.1                                                | High     | Fix         | Clarity is key.                                                                                                                  |
| **FF-04**  | The system shall allow users to navigate from a rule violation message in the Feedback Area to the corresponding location in the Score View. | `UserTesting.md` 5.4.1, 13.1.2                                                           | High     | Enhancement | Improves usability for addressing errors.                                                                                        |
| **FF-05**  | The Feedback Area shall update in real-time or near real-time as the user modifies the score and rules are re-validated. | `UserTesting.md` 12.2                                                                    | High     | Fix         | Requires efficient update mechanism.                                                                                             |
| **FF-06**  | The system shall provide analysis of voice characteristics (e.g., range, melodic contour).                             | `SRS_V3.md` 2.1.4 (Historical mistakes), `RequirementsAnalysis.md` 6, `UserTesting.md` 5.4.2 | Medium   | Enhancement | Requires Voice Analysis implementation (`TechnicalSpecification.md` 2.2). Display in Feedback Area (e.g., dedicated tab). |
| **FF-07**  | The system shall provide analysis of harmonic intervals between voice pairs.                                           | `SRS_V3.md` 2.1.4 (Historical mistakes), `UserTesting.md` 5.4.3                          | Medium   | Enhancement | Display in Feedback Area (e.g., dedicated tab).                                                                                  |
| **FF-08**  | The system shall provide analysis of rhythmic patterns or density (Details TBD).                                       | `SRS_V3.md` 3.1.3 (Rhythmic variety)                                                     | Low      | Enhancement | Specific metrics need definition.                                                                                                |
| **FF-09**  | The system should provide suggestions for improvement based on detected rule violations or analysis results.           | `SRS_V3.md` 2.1.4, 5.2.1, `UserTesting.md` 12.2                                          | Medium   | Enhancement | Suggestions could range from simple fixes to stylistic advice.                                                                   |
| **FF-10** | The system should provide historical analysis of common mistakes or patterns across multiple exercises (Progress Tracking). | `SRS_V3.md` 2.1.4, 5.2.2                                                                 | Low      | Enhancement | Requires data persistence and analysis logic. Links to FE-09.                                                                    |
| **FF-11** | The system shall visually distinguish different voices clearly in analysis displays (where applicable).                | `RequirementsAnalysis.md` 6                                                              | Medium   | Enhancement | Important for multi-voice analysis clarity.                                                                                      |
| **FF-12** | The system shall analyze and potentially identify voice roles (e.g., Soprano, Alto, Tenor, Bass) based on range/behavior. | `RequirementsAnalysis.md` 6, `TechnicalSpecification.md` 2.2                             | Low      | Enhancement | Requires pattern analysis.                                                                                                       |
| **FF-13** | The system shall analyze and potentially identify relationships between voices (e.g., imitation, contrary motion dominance). | `RequirementsAnalysis.md` 6, `TechnicalSpecification.md` 2.2                             | Low      | Enhancement | Requires interval and pattern analysis between voices.                                                                           |

#### 3.1.8. Solution and Hint System

These requirements define the system for providing users with assistance in completing exercises.

| Req ID   | Requirement Description                                                                                                | Source                                                                                   | Priority | Status      | Notes                                                                                                                               |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **FS-01**  | The system shall provide a mechanism for the user to request hints while working on an exercise.                       | `SRS_V3.md` 2.1.3, `UserTesting.md` 5.3.3                                                | High     | Fix         | Requires UI element (e.g., Hint button) and connection to hint generation logic. Needs implementation.                              |
| **FS-02**  | The system shall generate hints based on the current state of the user's score and the active rule set.                | `RequirementsAnalysis.md` 2, `UserTesting.md` 5.3.3                                      | High     | Enhancement | Hints should be context-aware.                                                                                                      |
| **FS-03**  | The system shall provide progressive hints, offering increasing levels of detail or directness upon repeated requests. | `RequirementsAnalysis.md` 2, `UserTesting.md` 5.3.3, 13.1.3                              | Medium   | Enhancement | E.g., Level 1: General direction, Level 2: Possible intervals, Level 3: Specific note suggestion.                               |
| **FS-04**  | Hint generation shall identify valid next notes or corrections based on the Rule Engine's validation logic (FR-12).    | `RequirementsAnalysis.md` 2, `TechnicalSpecification.md` 1.2                             | High     | Enhancement | Requires `SolutionGenerator` using constraint solving or search (A* mentioned in `TechnicalSpecification.md`).                |
| **FS-05**  | The system shall provide a mechanism for the user to request a complete solution for the current exercise.             | `SRS_V3.md` 2.1.3 (Solution checker), `RequirementsAnalysis.md` 2                        | Medium   | Enhancement | UI TBD (e.g., "Show Solution" button).                                                                                              |
| **FS-06**  | The system shall be capable of generating at least one valid solution for a given exercise template and rule set.      | `RequirementsAnalysis.md` 2, `TechnicalSpecification.md` 1.2, 5.1                        | High     | Enhancement | Solution must adhere to all active rules.                                                                                           |
| **FS-07**  | Generated solutions should aim for musical plausibility, not just rule adherence (where feasible).                     | `RequirementsAnalysis.md` 2                                                              | Low      | Enhancement | May require more advanced generation algorithms or heuristics.                                                                      |
| **FS-08**  | Solution generation performance must be reasonable, avoiding long delays for the user.                                 | `RequirementsAnalysis.md` 2, `TechnicalSpecification.md` 3.3                             | Medium   | Enhancement | Real-time suggestions are ideal but may be challenging. Background generation might be needed for complex cases.                |
| **FS-09**  | The system shall display generated solutions clearly in the Score View, potentially distinct from user input.          | `RequirementsAnalysis.md` 2                                                              | Medium   | Enhancement | Visual distinction TBD (e.g., different color, separate overlay).                                                                   |
| **FS-10** | The system shall provide example solutions for built-in exercises (FE-15).                                             | `SRS_V3.md` 7.2.4                                                                        | Medium   | Baseline    | These could be pre-generated or generated on demand.                                                                                |

#### 3.1.9. UI Controls and General Interaction

These requirements define the main user interface controls and general application interactions.

| Req ID   | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :--------------------------------------------------------------------------------------------------- |
| **FU-01**  | The application shall present a main window containing the primary interface elements.                                 | `SRS_V3.md` 2.1                                                        | High     | Baseline    | Standard application window.                                                                         |
| **FU-02**  | The main window shall include a Score View panel (See 3.1.1).                                                          | `SRS_V3.md` 2.1.1                                                        | High     | Baseline    |                                                                                                      |
| **FU-03**  | The main window shall include a Control Panel for playback and core functions.                                         | `SRS_V3.md` 2.1.2, `UserTesting.md` 5.1.1                              | High     | Baseline    | Purpose clear in testing.                                                                            |
| **FU-04**  | The Control Panel shall include transport controls: Play, Stop, Record.                                                | `SRS_V3.md` 2.1.2, `UserTesting.md` 5.1.3                              | High     | Fix         | Requires connection to MIDI playback/recording logic (FM-10).                                        |
| **FU-05**  | The Control Panel shall include a tempo control (e.g., slider or input field) allowing adjustment within a defined range (e.g., 40-208 BPM). | `SRS_V3.md` 2.1.2, `UserTesting.md` 5.1.3                              | High     | Fix         | Requires implementation and connection to playback engine. Range from `SRS_V3.md`.                   |
| **FU-06**  | The Control Panel shall include a metronome toggle button.                                                             | `SRS_V3.md` 2.1.2, `UserTesting.md` 5.1.3                              | High     | Fix         | Requires connection to metronome logic (FM-09).                                                      |
| **FU-07**  | The Control Panel shall provide controls for muting and soloing individual voices during playback (FM-08).             | `SRS_V3.md` 2.1.2                                                        | High     | Baseline    |                                                                                                      |
| **FU-08**  | The Control Panel shall display the current playback position (e.g., measure:beat).                                    | Implied by transport controls                                          | Medium   | Enhancement | Standard DAW/sequencer feature.                                                                      |
| **FU-09**  | The main window shall include an Exercise Panel displaying information about the current exercise (FE-08).             | `SRS_V3.md` 2.1.3, `UserTesting.md` 5.1.1                              | High     | Fix         | Purpose needs clarification via UI design (label, tooltip). Requires implementation.                 |
| **FU-10** | The Exercise Panel shall display rule violation indicators (summary or count).                                         | `SRS_V3.md` 2.1.3                                                        | High     | Fix         | Links to Feedback Area (FF-01) and Rule Engine (FR-03).                                              |
| **FU-11** | The Exercise Panel shall include controls for interacting with the exercise (e.g., Hint button FS-01, Solution button FS-05). | `SRS_V3.md` 2.1.3                                                        | High     | Fix         | Requires implementation.                                                                             |
| **FU-12** | The main window shall include a Feedback Area panel (See 3.1.7).                                                       | `SRS_V3.md` 2.1.4, `UserTesting.md` 5.1.1                              | High     | Fix         | Purpose needs clarification via UI design (label, tooltip). Requires implementation.                 |
| **FU-13** | The application shall provide a Settings Dialog accessible from the main window (e.g., via menu).                      | `SRS_V3.md` 2.2.2                                                        | High     | Baseline    | Standard application pattern.                                                                        |
| **FU-14** | The Settings Dialog shall allow configuration of MIDI input/output devices (FM-01, FM-02).                             | `SRS_V3.md` 2.2.2                                                        | High     | Baseline    |                                                                                                      |
| **FU-15** | The Settings Dialog shall allow configuration of audio settings (if applicable, e.g., buffer size, output device for metronome). | `SRS_V3.md` 2.2.2                                                        | Medium   | Baseline    | Details TBD based on audio engine implementation.                                                    |
| **FU-16** | The Settings Dialog shall allow configuration of UI preferences (e.g., voice colors FV-02, theme TBD).                 | `SRS_V3.md` 2.2.2                                                        | Low      | Enhancement |                                                                                                      |
| **FU-17** | The Settings Dialog shall provide access to Preset management (FP-08).                                                 | `SRS_V3.md` 2.2.2 (Rule set management)                                | High     | Enhancement |                                                                                                      |
| **FU-18** | The application shall provide standard File menu options (New Exercise, Open Exercise, Save Exercise, Save As..., Exit). | Standard Application Practice                                          | High     | Baseline    | Requires definition of exercise file format (See Data Management).                                   |
| **FU-19** | The application shall provide standard Edit menu options (Undo FN-10, Redo FN-10, Cut/Copy/Paste TBD).                 | Standard Application Practice                                          | High     | Enhancement | Cut/Copy/Paste for notes TBD.                                                                        |
| **FU-20** | UI elements should have tooltips explaining their function.                                                            | `UserTesting.md` 5.1.1, 13.1.4                                           | Medium   | Enhancement | Addresses usability feedback.                                                                        |
| **FU-21** | The application should provide a first-time use guided tour explaining the main UI panels.                             | `UserTesting.md` 5.1.1, 13.1.4                                           | Medium   | Enhancement | Addresses usability feedback.                                                                        |

#### 3.1.10. Development and Testing Support Interface

These requirements facilitate automated testing and interaction, particularly for agent-based development.

| Req ID   | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **FDV-01** | The application shall provide a command-line interface (CLI) or console mode allowing interaction with core functions.   | User Feedback                                                          | High     | Enhancement | Essential for agent interaction without GUI automation.                                                                             |
| **FDV-02** | The CLI shall support commands for loading exercises/scores.                                                           | Derived from FDV-01                                                    | High     | Enhancement | E.g., `load_exercise <path>`.                                                                                                       |
| **FDV-03** | The CLI shall support commands for adding/editing/deleting notes programmatically.                                     | Derived from FDV-01                                                    | High     | Enhancement | E.g., `add_note <voice> <pitch> <time> <duration> <velocity>`.                                                                      |
| **FDV-04** | The CLI shall support commands for invoking rule validation on the current score.                                      | Derived from FDV-01                                                    | High     | Enhancement | E.g., `validate_score`.                                                                                                             |
| **FDV-05** | The CLI shall support commands for retrieving validation results/feedback in a structured format (e.g., JSON, plain text). | Derived from FDV-01, FDV-06                                            | High     | Enhancement | E.g., `get_feedback`. Output format needs definition.                                                                               |
| **FDV-06** | The application shall produce structured logs (e.g., JSON format) containing detailed information about internal state changes, events, errors, and validation results. | User Feedback, Derived from NF-SUP-04                                | High     | Enhancement | Facilitates automated parsing by agents for debugging and state tracking. Supersedes basic logging requirement if implemented fully. |
| **FDV-07** | The application core logic (domain model, rule engine, MIDI processing if feasible) shall be runnable in a headless mode without initializing the GUI. | User Feedback                                                          | High     | Enhancement | Allows running tests and interactions in environments without a display server.                                                       |
| **FDV-08** | The CLI shall support commands for querying the current application state (e.g., loaded score summary, active ruleset). | Derived from FDV-01                                                    | Medium   | Enhancement | E.g., `get_state`.                                                                                                                  |

*(End of Functional Requirements Section 3.1)*

### 3.2. Non-Functional Requirements

This section specifies the non-functional requirements, which define the quality attributes and constraints of the system.

#### 3.2.1. Performance Requirements

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-PERF-01** | MIDI input to audio/MIDI output latency shall be less than or equal to 10 milliseconds (ms).                         | `SRS_V3.md` 6.1, `UserTesting.md` 5.2.2, 6.1, 9.2                      | High     | Baseline    | Critical for real-time interaction.                                                                                                 |
| **NF-PERF-02** | The Score View UI rendering frame rate shall be maintained at a minimum of 60 Frames Per Second (FPS) during typical interaction (scrolling, zooming, note editing). | `SRS_V3.md` 6.1, `UserTesting.md` 6.2, 9.2                      | High     | Baseline    | Ensures smooth visual experience. "Typical interaction" excludes extreme stress tests.                                              |
| **NF-PERF-03** | The application's memory usage shall not exceed 512MB during typical operation with moderately complex scores.       | `SRS_V3.md` 6.1, `UserTesting.md` 9.2                                  | Medium   | Baseline    | Target from V3, may need re-evaluation based on Qt6 and new features. "Moderately complex" TBD (e.g., < 100 measures, 4 voices). |
| **NF-PERF-04** | Real-time rule validation feedback (visual update in Score View and Feedback Area) shall appear within 50ms of user input causing a change. | `UserTesting.md` 9.2                                                 | High     | Baseline    | Ensures immediate feedback loop.                                                                                                    |
| **NF-PERF-05** | Application startup time (from launch to interactive main window) shall be less than 3 seconds on target hardware.   | `SRS_V3.md` 6.1                                                        | Medium   | Baseline    | Target hardware TBD (e.g., meeting minimum system specs).                                                                           |
| **NF-PERF-06** | Loading a moderately complex exercise file shall complete within 2 seconds.                                        | Derived from `UserTesting.md` 6.2 (Score load time)                    | Medium   | Baseline    | Definition of "moderately complex" TBD.                                                                                             |
| **NF-PERF-07** | Solution generation for typical species counterpoint exercises shall complete within a reasonable time frame (e.g., < 5 seconds). | Derived from `RequirementsAnalysis.md` 2, `TechnicalSpecification.md` 3.3 | Medium   | Enhancement | Avoids excessive waiting. "Typical" exercise TBD.                                                                                   |
| **NF-PERF-08** | Viewport grid expansion shall occur without noticeable stutter or delay during navigation.                           | Derived from `UserTesting.md` 6.2, `TechnicalSpecification.md` 3.4     | High     | Baseline    | Requires efficient background loading/expansion mechanisms.                                                                         |

#### 3.2.2. Usability Requirements

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-USAB-01** | The purpose of primary UI panels (Score View, Control Panel, Exercise Panel, Feedback Area) shall be clear and understandable to new users. | `UserTesting.md` 5.1.1                                                 | High     | Fix         | Addressed via FU-20 (Tooltips), FU-21 (Guided Tour), and clear labeling.                                                            |
| **NF-USAB-02** | Navigation within the Score View (scrolling, zooming, jumping) shall be intuitive and efficient.                     | `UserTesting.md` 5.1.2                                                 | High     | Fix         | Addressed via FV-08, FV-09, FV-10, FV-11. Standard interaction patterns expected.                                                    |
| **NF-USAB-03** | Note input and editing operations shall be discoverable and follow common interaction patterns.                      | `UserTesting.md` 5.2.1, 5.2.3                                          | High     | Fix         | Addressed via FN requirements and FV-14 (hover feedback).                                                                           |
| **NF-USAB-04** | Feedback on rule violations shall be easy to understand and locate within the score.                                 | `UserTesting.md` 5.4.1                                                 | High     | Fix         | Addressed via FF-03, FF-04.                                                                                                         |
| **NF-USAB-05** | The process for selecting, loading, and managing exercises shall be straightforward.                                 | `UserTesting.md` 5.3.1                                                 | High     | Fix         | Addressed via FE-05, FE-06, FE-07.                                                                                                  |
| **NF-USAB-06** | The process for selecting, customizing, and managing presets shall be straightforward.                               | Derived from `RequirementsAnalysis.md` 1                               | High     | Enhancement | Addressed via FP requirements.                                                                                                      |
| **NF-USAB-07** | The application shall provide mechanisms to prevent accidental data loss (e.g., confirmation dialogs for deletion, undo/redo FN-10). | Standard Usability Practice                                          | High     | Baseline    |                                                                                                                     |
| **NF-USAB-08** | Error messages shall be informative and suggest corrective actions where possible.                                   | `TechnicalSpecification.md` 6                                          | Medium   | Baseline    | Avoid technical jargon where possible.                                                                                              |

#### 3.2.3. Reliability Requirements

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-REL-01**  | The application shall handle unexpected MIDI device disconnection/reconnection without crashing.                     | `TechnicalSpecification.md` 6.2                                        | Medium   | Enhancement | Addressed via FM-12. May require user notification.                                                                                 |
| **NF-REL-02**  | The application shall handle invalid or corrupted exercise/preset files gracefully (e.g., error message, no crash).    | Derived from Persistence requirements                                  | Medium   | Baseline    |                                                                                                                     |
| **NF-REL-03**  | The application should recover from non-fatal errors without requiring a restart where possible.                     | `TechnicalSpecification.md` 6.2                                        | Medium   | Baseline    | E.g., state recovery, resource cleanup.                                                                                             |
| **NF-REL-04**  | The undo/redo history shall remain consistent and reliable during typical editing sessions.                          | Derived from FN-10                                                     | High     | Enhancement |                                                                                                                     |

#### 3.2.4. Supportability & Maintainability Requirements

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-SUP-01**  | The codebase shall adhere to defined coding standards (TBD).                                                         | Standard Practice                                                      | High     | Baseline    | Promotes consistency and readability.                                                                                               |
| **NF-SUP-02**  | The architecture shall remain modular, adhering to the layers defined in `TechnicalSpecification.md`.                | `TechnicalSpecification.md` High-Level Overview                        | High     | Baseline    | Facilitates independent development and testing.                                                                                    |
| **NF-SUP-03**  | The rule engine shall be extensible via plugins (FR-10).                                                             | `productContext.md` 26, `TechnicalSpecification.md` 2.10               | Medium   | Baseline    | Allows adding new rules without modifying core engine.                                                                              |
| **NF-SUP-04**  | The application shall include a logging mechanism for debugging and error reporting.                                 | `TechnicalSpecification.md` 8.2                                        | Medium   | Baseline    | Log levels and content TBD.                                                                                                         |
| **NF-SUP-05**  | Automated tests (unit, integration) shall be implemented for core components (Rule Engine, Domain Model, etc.), designed for execution in CI/CD pipelines and by automated agents. | `TechnicalSpecification.md` 7, User Feedback                           | High     | Baseline    | Ensures stability, facilitates refactoring, and supports agent-based testing.                                                       |
| **NF-SUP-06**  | Core components shall be designed with clear interfaces and minimal side effects to maximize testability by automated agents. | User Feedback                                                          | High     | Enhancement | Promotes deterministic behavior needed for reliable automated testing.                                                              |

#### 3.2.5. Compatibility Requirements

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-COMP-01** | The application shall run on Windows 10 and Windows 11 (64-bit).                                                     | `SRS_V3.md` 6.2                                                        | High     | Baseline    |                                                                                                                     |
| **NF-COMP-02** | The application shall run on common Linux distributions (e.g., Ubuntu 20.04+, Debian 11+) with required dependencies (ALSA, C++17, CMake). | `README.md`                                                            | High     | Baseline    | Specific distributions may require testing.                                                                                         |
| **NF-COMP-03** | The application shall be compatible with standard MIDI 1.0 compliant devices via USB or native MIDI ports.           | `SRS_V3.md` 6.2                                                        | High     | Baseline    |                                                                                                                     |
| **NF-COMP-04** | The application shall be built using Qt 6 (specific minimum version TBD).                                            | `SRS_V3.md` (Note), `README.md`, `productContext.md` 34                | High     | Baseline    |                                                                                                                     |
| **NF-COMP-05** | (Future) The application may need to support VST3 hosting or operate as a VST3 plugin.                               | `SRS_V3.md` 6.2, 8.2.1                                                 | Low      | Future      | Out of scope for initial V4 redesign.                                                                                               |
| **NF-COMP-06** | (Future) The application may need to import/export Standard MIDI Files (SMF).                                        | `SRS_V3.md` 6.2                                                        | Low      | Future      | Out of scope for initial V4 redesign unless required for custom exercises (FE-14).                                                  |

### 3.3. Interface Requirements

This section defines requirements related to how the software interacts with users and external systems/hardware.

#### 3.3.1. User Interfaces (UI)

This section describes the graphical user interface elements and interactions. Specific functional requirements for UI controls are detailed in Section 3.1.9.

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-UI-01** | The application shall provide a Graphical User Interface (GUI) based on the Qt 6 framework.                            | `SRS_V3.md` (Note), `README.md`, `productContext.md` 34                | High     | Baseline    |                                                                                                                     |
| **NF-UI-02** | The main application window shall be organized into logical panels: Score View, Control Panel, Exercise Panel, Feedback Area. | `SRS_V3.md` 2.1, `UserTesting.md` 5.1.1                              | High     | Baseline    | Layout should be clean and intuitive. Panel visibility/resizing TBD.                                                                |
| **NF-UI-03** | Visual feedback for user actions (e.g., note selection, button presses) shall be immediate and clear.                  | Standard UI Practice                                                   | High     | Baseline    |                                                                                                                     |
| **NF-UI-04** | Visual indicators for rule violations (FR-03) shall be clearly distinguishable within the Score View.                  | `SRS_V3.md` 2.1.4, `UserTesting.md` 5.3.2                              | High     | Fix         | Method TBD (e.g., note highlighting, icons, overlays).                                                                              |
| **NF-UI-05** | Different voices shall be visually distinct in the Score View using color (FV-02).                                     | `SRS_V3.md` 2.1.1, 12.1                                                  | High     | Baseline    | Colors should be configurable (FU-16).                                                                                              |
| **NF-UI-06** | The UI shall adhere to platform-specific human interface guidelines where appropriate (e.g., standard menu placement, dialog behavior). | Standard UI Practice                                                   | Medium   | Baseline    | For Windows and Linux.                                                                                              |
| **NF-UI-07** | The UI shall support standard keyboard shortcuts for common actions (e.g., Save, Open, Undo/Redo, Delete).            | Standard UI Practice                                                   | High     | Baseline    | Specific shortcuts TBD (e.g., FN-04, FN-10, FU-18, FU-19).                                                                          |
| **NF-UI-08** | The UI layout should adapt reasonably to different screen resolutions (minimum resolution TBD).                        | Standard UI Practice                                                   | Medium   | Baseline    | Responsive design principles.                                                                                                       |
| **NF-UI-09** | Font sizes and UI elements should be legible.                                                                          | Standard Usability Practice                                          | High     | Baseline    | Consider accessibility (though full accessibility compliance is not explicitly scoped yet).                                         |

#### 3.3.2. Hardware Interfaces

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-HI-01** | The system shall interface with MIDI input devices compliant with the MIDI 1.0 standard via USB or native MIDI ports.  | `SRS_V3.md` 6.2, NF-COMP-03                                            | High     | Baseline    | See Section 3.1.3 for functional MIDI requirements.                                                                                 |
| **NF-HI-02** | The system shall interface with MIDI output devices compliant with the MIDI 1.0 standard via USB or native MIDI ports. | `SRS_V3.md` 6.2, NF-COMP-03                                            | High     | Baseline    | See Section 3.1.3 for functional MIDI requirements.                                                                                 |

#### 3.3.3. Software Interfaces

| Req ID     | Requirement Description                                                                                                | Source                                                                 | Priority | Status      | Notes                                                                                                                               |
| :--------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **NF-SI-01** | (Future) The system may interface with VST3 host applications (DAWs) if implemented as a VST3 plugin.                | `SRS_V3.md` 6.2, 8.2.1, NF-COMP-05                                     | Low      | Future      | Requires implementing the VST3 SDK interfaces. Out of scope for initial V4 redesign.                                                  |
| **NF-SI-02** | (Future) The system may need to interface with operating system file systems for importing/exporting Standard MIDI Files (SMF). | `SRS_V3.md` 6.2, NF-COMP-06                                            | Low      | Future      | Requires SMF parsing/writing library/logic. Out of scope for initial V4 redesign unless needed for custom exercises (FE-14). |

---

## 4. Data Management Requirements

This section defines requirements related to the storage, retrieval, and management of application data.

| Req ID   | Requirement Description                                                                                                | Source                                                                                   | Priority | Status      | Notes                                                                                                                               |
| :------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------- | :------- | :---------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| **FD-01**  | The system shall persist user settings (e.g., selected MIDI devices, UI preferences) across application sessions.        | `SRS_V3.md` 7.1.4                                                                        | High     | Baseline    | Storage mechanism TBD (e.g., config file, registry).                                                                                |
| **FD-02**  | The system shall persist user-created custom presets (FP-04).                                                          | `RequirementsAnalysis.md` 1                                                              | High     | Enhancement | Requires defining a preset file format (JSON mentioned in `TechnicalSpecification.md` 2.1). Location TBD (e.g., user documents). |
| **FD-03**  | The system shall store built-in exercises (FE-15) within the application installation or resource files.               | `SRS_V3.md` 7.2.1                                                                        | High     | Baseline    | Format TBD. Must be readable by the application.                                                                                    |
| **FD-04**  | The system shall allow users to save their work on an exercise to a file (FU-18).                                      | `SRS_V3.md` 7.1.1 (Implied by progress), Standard Practice                               | High     | Baseline    | Requires defining an exercise/score file format.                                                                                    |
| **FD-05**  | The system shall allow users to open previously saved exercise files (FU-18).                                          | Standard Practice                                                                        | High     | Baseline    |                                                                                                                     |
| **FD-06** | The defined exercise/score file format shall store all necessary information to reconstruct the state (notes, voices, time/key signatures, associated preset/rules, user progress state TBD). | Derived Requirement                                                                      | High     | Baseline    | Completeness is key for save/load functionality.                                                                                    |
| **FD-07** | (Future) The system may persist user progress data (completed exercises, scores, common mistakes) for tracking and analysis (FE-09, FF-10). | `SRS_V3.md` 7.1.1, 7.1.3                                                                 | Low      | Future      | Requires defining data structures and storage mechanism. Out of scope for initial V4 redesign.                                        |
| **FD-08** | (Future) The system may support importing/exporting data in standard formats like Standard MIDI File (SMF) or MusicXML. | `SRS_V3.md` 6.2, NF-COMP-06                                                              | Low      | Future      | Out of scope for initial V4 redesign.                                                                                               |

---

## 5. Prioritization

Requirements identified in this document have been assigned a priority level (High, Medium, Low) based on core functionality needs, user testing feedback (`UserTesting.md`), and feature analysis (`RequirementsAnalysis.md`). These priorities are intended to guide the development focus for the V4 redesign.

*   **High Priority:** Core functionality essential for a usable application, fixes for critical bugs identified in user testing, and foundational elements for other features. These should be addressed first. Examples include:
    *   Basic Score View rendering and navigation (FV-01 to FV-10).
    *   Functional Note Input via mouse and MIDI (FN-01, FN-02, FN-03).
    *   Basic Note Editing (Delete, Move Pitch/Time) (FN-04, FN-05, FN-06).
    *   Core MIDI device detection and playback (FM-01, FM-02, FM-03, FM-07, FM-10, FM-13).
    *   Basic Rule Engine validation and feedback loop (FR-01, FR-02, FR-03, FR-04, FR-05, FR-06).
    *   Exercise Browser and loading (FE-05, FE-06, FE-07, FE-08, FE-15).
    *   Basic Preset selection and application (FP-01, FP-02).
    *   Undo/Redo functionality (FN-10).
    *   Core UI Controls (Transport, Panels) (FU-01 to FU-07, FU-09, FU-12, FU-13, FU-14, FU-18).
    *   Critical Performance targets (NF-PERF-01, NF-PERF-02, NF-PERF-04, NF-PERF-08).
    *   Core Usability fixes (NF-USAB-01, NF-USAB-02, NF-USAB-03, NF-USAB-04, NF-USAB-05).
    *   Basic Reliability and Supportability (NF-REL-02, NF-SUP-01, NF-SUP-02, NF-SUP-05).
    *   Core Compatibility (NF-COMP-01, NF-COMP-02, NF-COMP-03, NF-COMP-04).
    *   Basic Data Management (FD-01, FD-04, FD-05, FD-06).

*   **Medium Priority:** Important features that enhance usability or functionality but are not strictly required for a basic working version. Includes enhancements identified in analysis and less critical fixes. Examples include:
    *   Advanced Score View features (Minimap FV-11, Velocity Viz FV-07).
    *   Advanced Note Editing (Duration FN-07, Property Editor FN-08, Step Recording FN-09).
    *   Advanced MIDI features (Sustain FM-05, Multi-channel FM-06, Channel Mapping FM-11, Graceful Disconnect FM-12).
    *   Advanced Rule features (Cadences FR-07c, Configurable Params FR-09, Conflict Detection FR-11).
    *   Learning Paths and Custom Exercises (FE-10 to FE-14, FE-17).
    *   Full Preset Management (Create, Save, Load, Delete, Validate) (FP-03 to FP-09).
    *   Voice and Interval Analysis (FF-06, FF-07, FF-11).
    *   Improvement Suggestions (FF-09).
    *   Basic Solution/Hint System (FS-01, FS-02, FS-04, FS-05, FS-06, FS-08, FS-09, FS-10).
    *   Additional UI Controls and Polish (Playback Position FU-08, Tooltips FU-20, Guided Tour FU-21, Settings FU-15, FU-17, Edit Menu FU-19).
    *   Other Performance targets (NF-PERF-03, NF-PERF-05, NF-PERF-06, NF-PERF-07).
    *   Other Reliability aspects (NF-REL-01, NF-REL-03).
    *   Logging (NF-SUP-04).

*   **Low Priority:** Desirable features, "nice-to-haves", or features planned for future iterations beyond the initial V4 redesign scope. Examples include:
    *   Advanced Rhythmic/Style Analysis (FF-08, FF-12, FF-13).
    *   Historical Mistake Analysis (FF-10).
    *   Musically Plausible Solutions (FS-07).
    *   Progress Tracking / Gamification (FE-09, FE-16).
    *   Advanced UI Preferences (FU-16).
    *   Preset Hot-Reloading (FP-11).
    *   VST3 / SMF / MusicXML support (NF-COMP-05, NF-COMP-06, FD-08).
    *   User Progress Data Persistence (FD-07).

Priorities may be revisited during development based on effort estimation, dependencies, and further feedback. The "Priority" column in the detailed requirement tables reflects these levels.

---

## 6. Future Considerations / Removed Features

This section lists features or requirements that are explicitly considered out of scope for the initial MusicTrainer V4 redesign, potentially to be addressed in future versions. Many of these are derived from `SRS_V3.md` Section 8 or marked as "Low Priority" or "Future" in the detailed requirements sections above.

*   **Advanced AI/ML Features:**
    *   AI-powered exercise generation (`SRS_V3.md` 8.1.1).
    *   Advanced pattern recognition beyond basic melodic/harmonic analysis (`RequirementsAnalysis.md` Low Priority).
    *   Style-based generation/analysis (`RequirementsAnalysis.md` Low Priority).

*   **Collaboration and Cloud Features:**
    *   Real-time collaboration (`SRS_V3.md` 8.1.2).
    *   Cloud synchronization of settings, presets, or progress (`SRS_V3.md` 8.1.3).

*   **Platform Expansion:**
    *   Mobile companion app (`SRS_V3.md` 8.1.4).
    *   Web-based version (`SRS_V3.md` 8.2.2).
    *   Official macOS support (currently secondary constraint).

*   **Advanced Integration:**
    *   DAW plugin version (VST3) (NF-COMP-05, `SRS_V3.md` 8.2.1).
    *   Standard MIDI File (SMF) import/export (NF-COMP-06, FD-08).
    *   MusicXML import/export (FD-08).

*   **Educational/Institutional Features:**
    *   Specific features for educational institutions (`SRS_V3.md` 8.2.3).
    *   Professional certification tracking (`SRS_V3.md` 8.2.4).

*   **Advanced Data/Analysis:**
    *   Detailed historical progress tracking and analysis persistence (FD-07, FF-10).

*   **Removed/Superseded V3 Features (Implicit):**
    *   Any specific V3 requirements not carried over, enhanced, or replaced by requirements in this V4 document are considered removed or superseded. (Requires final review comparison).

*   **Other Low Priority Items:** Refer to items marked "Low" priority in the detailed requirement tables (Sections 3.1, 3.2, 3.3, 4).

---

## 7. Appendices

This section contains supplementary information referenced in the SRS.

*(Appendices such as a full Glossary, detailed analysis models, or specific rule set examples may be added here as needed during development.)*