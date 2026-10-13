# MusicTrainer — PyQt6 Frontend

This directory contains the **PyQt6** GUI for MusicTrainer. The musical logic
(domain model, rule validation, MIDI I/O, command/undo-redo) lives in the
**Qt-free C++ core** and is consumed through the compiled `musictrainer_core`
pybind11 extension module. For the rationale and migration plan, see
[`docs/current/PyQtMigration.md`](../docs/current/PyQtMigration.md).

## Layout

```
python/
  pyproject.toml            # setuptools packaging (src/ layout)
  requirements.txt          # PyQt6
  README.md
  src/musictrainer/
    __init__.py
    __main__.py             # `python -m musictrainer` entry point
    app.py                  # QApplication bootstrap
    core.py                 # re-export of the compiled musictrainer_core module
    main_window.py          # MainWindow (PyQt port of the former C++ UI)
    score_viewport.py       # Piano-roll viewport widget (PyQt)
```

> **Status:** scaffold. This pass establishes the package, the pybind11 bridge,
> and a runnable PyQt shell. Full interaction (drag/resize, selection, playback,
> MIDI input callbacks) is implemented incrementally on top of it.

## Build & run

### 1. Build the C++ core + Python bindings

From the repository root (not this directory):

```bash
cmake -S . -B build -DMUSICTRAINER_BUILD_PYTHON_BINDINGS=ON
cmake --build build --target musictrainer_core
```

The bindings target copies `musictrainer_core.{pyd,so}` into
`python/src/musictrainer/` automatically. If your CMake version/output layout
differs, copy the built extension there (or set `PYTHONPATH`).

### 2. Install the Python package

```bash
cd python
python -m pip install -e .
```

### 3. Run

```bash
python -m musictrainer
# or, after install:
musictrainer
```

`musictrainer_core` must be importable (on `sys.path`). If you see an
`ImportError` about `musictrainer_core`, complete step 1 and ensure the
extension sits next to this package.

## Notes

- The legacy C++/Qt GUI (`src/ui/`, `src/main.cpp`) still builds with
  `-DMUSICTRAINER_BUILD_QT_APP=ON` (the default) when Qt6 is present. It is
  being superseded by this frontend; see the migration doc.
- MIDI output works through `musictrainer_core.RtMidiSystem`. Input callbacks
  are not yet bound (tracked as a follow-up item).
