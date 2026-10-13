"""MusicTrainer PyQt6 frontend.

This package is the PyQt6 GUI for MusicTrainer. All musical logic (domain model,
rule validation, MIDI I/O, command/undo-redo) lives in the Qt-free C++ core and
is exposed to Python through the compiled ``musictrainer_core`` pybind11 extension
module (see ``bindings/`` and ``docs/current/PyQtMigration.md``).

Run with: ``python -m musictrainer`` (after building the bindings).
"""

__version__ = "0.1.0"
