# MusicTrainer

MusicTrainer is an application designed to help users improve their music theory skills through interactive exercises and real-time feedback.

## Features

- Piano-roll style score view for composition and notation
- Real-time music theory rule validation
- MIDI input/output for playing and recording
- Customizable exercises for different skill levels
- Detailed feedback on music theory concepts

## Dependencies

### Required Dependencies

- CMake 3.24 or higher
- C++17 compatible compiler
- ALSA development libraries (Linux only)

### Optional Dependencies

- Qt 6.0 or Qt 5.15 — **only** for the legacy C++/Qt GUI application
  (`MusicTrainerApp`). The core library and tests build without Qt.
- JACK audio connection kit (for additional MIDI functionality)
- Python 3.9+ and PyQt6 — for the **PyQt frontend** (see below). Building the
  Python bindings also requires pybind11, which CMake fetches automatically.

> **Frontend direction:** the GUI is being migrated from C++/Qt ("Qt proper") to
> **PyQt** on top of the Qt-free C++ core. The PyQt frontend lives in
> [`python/`](python/) and is described in
> [`docs/current/PyQtMigration.md`](docs/current/PyQtMigration.md).

## Building the Application

### Installing Dependencies

#### Ubuntu/Debian

```bash
# Install essential build tools
sudo apt update
sudo apt install build-essential cmake git

# Install ALSA development libraries
sudo apt install libasound2-dev

# Install Qt (prefer Qt 6 if available)
sudo apt install qt6-base-dev qt6-charts-dev
# If Qt 6 isn't available:
# sudo apt install qt5-default libqt5charts5-dev
```

#### macOS

```bash
# Install using Homebrew
brew install cmake
brew install qt6
# or for Qt5: brew install qt5
```

#### Windows

- Install [CMake](https://cmake.org/download/)
- Install [Qt](https://www.qt.io/download-qt-installer)
- Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) with C++ support

### Building the Application

```bash
# Clone the repository
git clone https://github.com/yourusername/MusicTrainer.git
cd MusicTrainer

# Create a build directory
mkdir -p build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build .
```

## Running the Application

### Legacy C++/Qt GUI (`MusicTrainerApp`)

Once built, you can find the executable in the `build/bin` directory (the GUI
requires Qt 5.15/6 to be built; otherwise only the test suite is built):

- `MusicTrainerApp`: The full graphical user interface (built only when Qt6 is found)

This target is being superseded by the PyQt frontend (below).

### PyQt Frontend (Python)

The PyQt6 GUI reuses the Qt-free C++ core through the `musictrainer_core`
pybind11 module:

```bash
cmake -S . -B build -DMUSICTRAINER_BUILD_PYTHON_BINDINGS=ON
cmake --build build --target musictrainer_core
cd python
python -m pip install -e .
python -m musictrainer
```

See [`python/README.md`](python/README.md) for full details.

## Getting Started

1. Start by running the main GUI application: `./build/bin/MusicTrainerApp`
2. Create a new exercise or open an existing one from the File menu
3. Use the transport controls at the bottom to play or record
4. Add notes by clicking on the score view
5. Get feedback on your composition in the Feedback panel

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.