# PyQt Migration Plan (Qt proper → PyQt)

**Status:** Scaffold pass complete. Full UI/bindings port is incremental follow-up.
**Related:** `ArchitectureDecisions.md` → ADR-011 (PyQt frontend over C++/Qt).

## 1. Motivation

The application currently ships a **C++/Qt6** GUI (`src/ui/MainWindow.*`,
`src/ui/ScoreViewportWidget.*`, `src/main.cpp`). The rest of the codebase — the
domain model, rule engine, MIDI I/O, loaders, and command/undo-redo system — is
**already Qt-free**. It forms the `MusicTrainer_Core` static library, which only
depends on `nlohmann_json` and `rtmidi`.

Moving the GUI to **PyQt** (the Python binding of Qt) lets us:

- Iterate on UI without recompiling C++ and without a full Qt toolchain on every
  machine (the core test suite already builds Qt-free on CI).
- Keep the validated, performance-sensitive core (MIDI latency, lock-free queues,
  rule validation) in C++ and reuse it as-is.
- Simplify distribution of the GUI layer and reduce the C++ surface area that
  touches Qt's meta-object compiler.

## 2. Target architecture

```
┌─────────────────────────┐      import       ┌──────────────────────────┐
│   PyQt6 GUI (Python)    │ ─────────────────▶ │  musictrainer_core       │
│  python/src/musictrainer│      (pybind11)     │  (pybind11 module)       │
│  MainWindow + Viewport  │ ◀───────────────── │  wraps MusicTrainer_Core │
└─────────────────────────┘   Score/Note/...   └──────────────────────────┘
                                                      │ links
                                                      ▼
                                            MusicTrainer_Core (C++)
                                            domain / rules / midi / io / commands
                                            (Qt-free)
```

- **C++ core:** unchanged, Qt-free, built as a static library.
- **pybind11 bindings:** `bindings/core_bindings.cpp` compiles to the importable
  module `musictrainer_core`. This is the only new C++ that depends on the core.
- **PyQt frontend:** `python/src/musictrainer/` — pure Python/PyQt6.

## 3. What this scaffold pass did

### Build hygiene (root `CMakeLists.txt`)
- Removed the **global** `CMAKE_AUTOMOC`/`AUTORCC`/`AUTOUIC` flags. They are now
  set **per-target** (`MusicTrainerApp` only), so non-Qt targets are unaffected.
- Added two options:
  - `MUSICTRAINER_BUILD_QT_APP` (default **ON**) — builds the legacy C++/Qt GUI
    only when both the option is on and Qt6 is found.
  - `MUSICTRAINER_BUILD_PYTHON_BINDINGS` (default **OFF**) — builds the
    `musictrainer_core` pybind11 module.
- The core library and the test suite remain buildable with **no Qt installed**.

### Python bindings scaffold (`bindings/`)
- `bindings/CMakeLists.txt` fetches pybind11 (`v2.12.0`, pinned) and builds the
  module, linking `MusicTrainer_Core` + `rtmidi`.
- `bindings/core_bindings.cpp` exposes the API the GUI needs:
  `Pitch`, `Position`, `Note`, `Score`/`Voice`, `TimeSignature`, `KeySignature`,
  `ConcreteRuleEngine`, `Preset`/`PresetLoader`, `ScoreLoader`, `RtMidiSystem`,
  `CommandHistory`, and command helpers (`add_note`, `delete_notes`,
  `move_notes`, `resize_note`, `register_default_rules`).

### PyQt6 frontend scaffold (`python/`)
- `pyproject.toml` (src layout), `requirements.txt` (PyQt6), and a README with
  build/run instructions.
- `core.py` — thin, friendly re-export of `musictrainer_core` with a clear error
  if the extension is not built.
- `score_viewport.py` — `QWidget` that paints the score from the bound `Score`
  and adds a note on click via the command history.
- `main_window.py` — ports the former C++ `MainWindow`: menus, toolbars, feedback
  and exercise docks, central viewport, open/save, undo/redo, and live validation
  feedback.
- `app.py` / `__main__.py` — launch via `python -m musictrainer`.

## 4. Known gaps / follow-up work

The binding and UI are intentionally **not complete**. Track these as follow-ups:

1. **Polymorphic rule registration from Python.** Only `ParallelFifthsRule` and
   `ParallelOctavesRule` are registered via `register_default_rules()`. Exposing
   `IRule` to Python for custom rule plugins is future work.
2. **MIDI input callbacks.** `RtMidiSystem::setInputCallback` (a `std::function`)
   is not bound yet; MIDI *output* works, *input* event routing needs a wrapper.
3. **Violation detail.** `Violation::involvedNotes` (raw `Note*`) is omitted;
   the scaffold surfaces `ruleId` / `description` / `severity` / `location`.
4. **Full viewport interactions.** Drag-move, resize, selection rectangle,
   playback cursor, zoom/scroll with wheel, and keyboard delete are not yet
   ported from the C++ widget.
5. **Playback engine.** Transport start/stop, timer-driven note scheduling, and
   metronome are stubbed, not implemented.
6. **Retire the C++/Qt UI.** Once the PyQt frontend is feature-complete,
   `src/ui/` and `src/main.cpp` (and the `MUSICTRAINER_BUILD_QT_APP` option)
   should be removed.

## 5. How to build and run

See [`python/README.md`](../../python/README.md) for the exact commands. Summary:

```bash
# From the repo root:
cmake -S . -B build -DMUSICTRAINER_BUILD_PYTHON_BINDINGS=ON
cmake --build build --target musictrainer_core

cd python
python -m pip install -e .
python -m musictrainer
```

## 6. Testing strategy

- The existing C++ GoogleTest suite (`tests/`) continues to validate the core;
  it is unaffected by the PyQt work.
- New Python-side tests should validate the bridge (e.g. load a `.mtx`, validate,
  assert violation count) and live under `python/tests/` as a follow-up.
