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

- Qt 6.0 or Qt 5.15 (for GUI application)
- JACK audio connection kit (for additional MIDI functionality)

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

Once built, you can find the executables in the `build/bin` directory:

- `MusicTrainerV3`: The main command-line application
- `MusicTrainerGUI`: The full graphical user interface (if Qt is available)
- `TestGUI`: A simpler test GUI for demonstration (if Qt is available)

## Getting Started

1. Start by running the main GUI application: `./build/bin/MusicTrainerGUI`
2. Create a new exercise or open an existing one from the File menu
3. Use the transport controls at the bottom to play or record
4. Add notes by clicking on the score view
5. Get feedback on your composition in the Feedback panel

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.