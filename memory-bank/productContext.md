# Product Context

This file provides a high-level overview of the project and the expected product that will be created. Initially it is based upon projectBrief.md (if provided) and all other available project-related information in the working directory. This file is intended to be updated as the project evolves, and should be used to inform all other modes of the project's goals and context.
2025-03-19 11:05:48 - Log of updates made will be appended as footnotes to the end of this file.

*

## Project Goal

*   Develop an advanced music theory training application specializing in counterpoint education, using interactive MIDI-based exercises, real-time feedback, and progressive learning paths.

## Key Features

*   Interactive counterpoint exercises (Species and Free Counterpoint)
*   Real-time MIDI input and output, with low latency (<= 10ms)
*   Progressive learning system (Beginner, Intermediate, Advanced paths)
*   Detailed visual feedback (piano roll display, rule violation indicators)
*   Customizable rule sets (melodic, harmonic, rhythmic)
*   Exercise history and analytics
*   Plugin architecture for extensibility

## Overall Architecture

*   Event-Driven Architecture for state management
*   Immutable Domain Objects for predictable state and thread safety
*   Plugin Architecture for rule system and exercise types
*   MIDI Processing with a lock-free queue and dedicated thread
*   Progressive Validation Strategy for UI responsiveness
*   MVVM pattern with reactive bindings for the UI
*   Hierarchical error handling with domain-specific errors

2025-03-19 11:14:15 - Updated Project Goal, Key Features, and Overall Architecture based on SRS_V3.md and ArchitectureDecisions.md.
*   Multi-layer testing strategy, including property-based testing
*   Implemented with Qt6