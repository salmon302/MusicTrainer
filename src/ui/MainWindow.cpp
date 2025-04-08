#include "MainWindow.h"
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog> // For open/save slots
#include <QMessageBox> // For placeholders
#include <QLabel>      // For placeholder dock content
#include <QDockWidget> // For dockable panels

// Include the actual viewport widget header
#include "ScoreViewportWidget.h"
#include "../rules/ConcreteRuleEngine.h" // Include concrete engine
#include "../rules/ParallelFifthsRule.h" // Include concrete rule
#include "../io/PresetLoader.h"       // Include preset loader
#include "../domain/Score.h"          // Include Score definition
#include "../rules/Preset.h"          // Include Preset definition
#include "../midi/RtMidiSystem.h"     // Include concrete MIDI system
#include "../midi/MidiSystem.h"       // Include MIDI interface and MidiEvent
#include <iostream>                   // For std::cerr, std::endl
#include <stdexcept>                  // For std::exception
#include <filesystem>                 // For path handling
#include <functional>                 // For std::bind, std::placeholders
#include "../commands/CommandHistory.h" // Include Command History
#include <QTimer>                     // For playback timer
#include <QToolBar>                   // For transport/tempo controls
#include <QSpinBox>                   // For tempo control
#include <set>                        // For tracking active playback notes
#include <QListWidget>                // For exercise browser
#include <QListWidgetItem>            // For exercise browser items
#include <QDir>                       // For scanning exercise directory

// #include "ControlPanelWidget.h"
// #include "FeedbackPanelWidget.h"
// #include "ExercisePanelWidget.h"

namespace MusicTrainer {
namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      currentScore(std::make_unique<Domain::Score>()),         // Initialize score
      ruleEngine(std::make_unique<Rules::ConcreteRuleEngine>()), // Initialize engine
      midiSystem(std::make_unique<Midi::RtMidiSystem>()),       // Initialize MIDI system
      commandHistory(std::make_unique<Commands::CommandHistory>()) // Initialize Command History
    // If using Qt Designer: , ui(new Ui::MainWindow)
{
    // If using Qt Designer: ui->setupUi(this);

    std::cout << "Initializing MainWindow..." << std::endl; // Debug

    // --- Initialize Core Components ---
    try {
        // 1. Register known rules with the engine
        ruleEngine->registerRulePlugin(std::make_shared<Rules::ParallelFifthsRule>());
        ruleEngine->registerRulePlugin(std::make_shared<Rules::ParallelOctavesRule>()); // Register the new rule
        // TODO: Register other rules here

        // 2. Load the default preset
        // TODO: Make preset path configurable or discoverable
        std::filesystem::path presetPath = "presets/default.preset.json";
        Rules::Preset defaultPreset = IO::PresetLoader::loadFromFile(presetPath);

        // 3. Load the preset into the engine
        ruleEngine->loadRuleSet(defaultPreset);
        std::cout << "Loaded default preset: " << defaultPreset.name << std::endl;

    } catch (const std::exception& e) {
         // Handle errors during engine setup/preset loading
         std::cerr << "ERROR initializing rule engine or loading preset: " << e.what() << std::endl;
         // Show error message to user?
         QMessageBox::critical(this, "Initialization Error",
                               QString("Failed to initialize core components: %1").arg(e.what()));
         // Application might be in an unusable state here
    }

    // --- Initialize MIDI System ---
    try {
        if (midiSystem) { // Check if midiSystem was initialized successfully
            auto inputs = midiSystem->listInputDevices();
            if (!inputs.empty()) {
                std::cout << "Available MIDI Inputs:" << std::endl;
                for(const auto& input : inputs) { std::cout << " - " << input << std::endl; }
                // Attempt to open the first available input device
                if (midiSystem->openInputDevice(inputs[0])) {
                    // Set the callback using std::bind to member function
                    midiSystem->setInputCallback(
                        std::bind(&MainWindow::handleMidiEvent, this, std::placeholders::_1)
                    );
                    statusBar()->showMessage(QString("Opened MIDI Input: %1").arg(QString::fromStdString(inputs[0])), 5000);
                } else {
                     statusBar()->showMessage("Failed to open default MIDI Input.", 5000);
                     // Optionally show warning message box
                }
            } else {
                 statusBar()->showMessage("No MIDI Input devices found.", 5000);
            }
            // TODO: Add similar logic for output device selection/opening if needed
        }
    } catch (const std::exception& e) {
         std::cerr << "ERROR initializing MIDI system: " << e.what() << std::endl;
         QMessageBox::warning(this, "MIDI Error",
                              QString("Failed to initialize MIDI system: %1").arg(e.what()));
    }


    // --- Setup UI ---
    createActions();
    createMenus();
    createToolBars(); // Call toolbar creation
    createStatusBar();
    createDockWidgets(); // This creates feedbackLabelPtr
    setupCentralWidget(); // This creates scoreViewport

    // --- Connect Core and UI ---
    // Pass the initial score to the viewport
    if (scoreViewport && currentScore) {
        scoreViewport->setScore(currentScore.get());
        scoreViewport->setMainWindow(this); // Give viewport a pointer back to main window
    }
    // Initial validation and feedback update
    validateScoreAndUpdateFeedback();

    // TODO: Connect signals/slots for score updates, validation requests, feedback display etc.
    // Example: connect(currentScore.get(), &Score::scoreChanged, this, &MainWindow::validateScoreAndUpdateFeedback);

    // --- Setup Playback Timer ---
    playbackTimer = new QTimer(this);
    // Connect timer timeout to the advancePlayback slot
    connect(playbackTimer, &QTimer::timeout, this, &MainWindow::advancePlayback);
    // Set a default interval (e.g., 16th note at 120bpm) - will be adjusted in play()
    // 120 bpm = 2 beats/sec. 16th note = 0.25 beats. Duration = 0.25 / 2 = 0.125 sec = 125 ms.
    // Let's use a smaller interval for better resolution, e.g., 50ms.
    playbackTimer->setInterval(50); // ms


    setWindowTitle("Music Trainer");
    // setMinimumSize(800, 600); // Set a reasonable default size
}

MainWindow::~MainWindow()
{
    // If using Qt Designer: delete ui;
    std::cout << "Destroying MainWindow..." << std::endl; // Debug
}

void MainWindow::createActions() {
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing score file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFileAction);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current score"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFileAction);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close); // Connect to QWidget::close

    // Play Action
    playAct = new QAction(tr("&Play"), this);
    // TODO: Add icon QIcon::fromTheme("media-playback-start")
    playAct->setShortcut(Qt::Key_Space); // Example shortcut
    playAct->setStatusTip(tr("Start playback from the beginning"));
    connect(playAct, &QAction::triggered, this, &MainWindow::play);
    // playAct->setEnabled(true); // Enable based on whether score exists/output open?

    // Stop Action
    stopAct = new QAction(tr("&Stop"), this);
    // TODO: Add icon QIcon::fromTheme("media-playback-stop")
    stopAct->setShortcut(Qt::Key_Space); // Example shortcut (might toggle)
    stopAct->setStatusTip(tr("Stop playback"));
    connect(stopAct, &QAction::triggered, this, &MainWindow::stop);
    stopAct->setEnabled(false); // Initially disabled

    // Undo Action
    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last action"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo); // Connect to slot
    undoAct->setEnabled(false); // Initially disabled

    // Redo Action
    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo); // Standard Redo shortcut
    redoAct->setStatusTip(tr("Redo the last undone action"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo); // Connect to slot
    redoAct->setEnabled(false); // Initially disabled

    // Metronome Action
    metronomeAct = new QAction(tr("Metronome"), this);
    // TODO: Add icon QIcon::fromTheme("preferences-system-time")
    metronomeAct->setCheckable(true);
    metronomeAct->setChecked(isMetronomeEnabled); // Set initial state
    metronomeAct->setStatusTip(tr("Toggle metronome click during playback"));
    connect(metronomeAct, &QAction::toggled, this, &MainWindow::toggleMetronome);

    // TODO: Add actions for Cut, Copy, Paste, Preferences etc.
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct); // Add Undo action
    editMenu->addAction(redoAct); // Add Redo action
    editMenu->addSeparator();
    // TODO: Add Cut, Copy, Paste, Preferences...

    viewMenu = menuBar()->addMenu(tr("&View"));
    // TODO: Add View actions (Show/Hide panels, Zoom...)

    helpMenu = menuBar()->addMenu(tr("&Help"));
    // TODO: Add Help actions (About, Documentation...)
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWidgets() {
    // Placeholder for dockable panels (Feedback, Exercise Info, etc.)

    // Example: Feedback Panel
    QDockWidget *feedbackDock = new QDockWidget(tr("Feedback"), this);
    feedbackDock->setObjectName("FeedbackDockWidget"); // Good practice for saving state
    feedbackDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    // Create the label and store the pointer
    feedbackLabelPtr = new QLabel(tr("Initializing...")); // Initial text
    feedbackLabelPtr->setWordWrap(true);
    feedbackLabelPtr->setAlignment(Qt::AlignTop);
    feedbackDock->setWidget(feedbackLabelPtr); // Set label as widget
    addDockWidget(Qt::RightDockWidgetArea, feedbackDock);
    viewMenu->addAction(feedbackDock->toggleViewAction()); // Add action to View menu

     // Exercise Browser Panel (FE-05)
    QDockWidget *exerciseDock = new QDockWidget(tr("Exercises"), this);
    exerciseDock->setObjectName("ExerciseDockWidget");
    exerciseDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    exerciseListWidget = new QListWidget(exerciseDock); // Create the list widget
    exerciseListWidget->setToolTip(tr("Double-click an exercise to load it."));
    // Populate the list widget
    QDir exerciseDir("exercises"); // Relative path to exercises directory
    QStringList nameFilters;
    nameFilters << "*.mtx"; // Filter for our exercise file extension
    QStringList exerciseFiles = exerciseDir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QString& fileName : exerciseFiles) {
        // Add just the filename without extension as the display text
        QFileInfo fileInfo(fileName);
        QListWidgetItem* item = new QListWidgetItem(fileInfo.baseName());
        // Store the full path in the item's data for later retrieval
        item->setData(Qt::UserRole, exerciseDir.absoluteFilePath(fileName));
        exerciseListWidget->addItem(item);
    }

    // Connect double click signal to load slot
    connect(exerciseListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::loadSelectedExercise);

    exerciseDock->setWidget(exerciseListWidget); // Set list widget as the dock's widget
    addDockWidget(Qt::LeftDockWidgetArea, exerciseDock);
    viewMenu->addAction(exerciseDock->toggleViewAction()); // Add action to View menu

    // TODO: Add filtering controls (FE-06, FE-07)
}

void MainWindow::setupCentralWidget() {
    // Replace placeholder with actual ScoreViewportWidget instance
    scoreViewport = new ScoreViewportWidget(this); // Instantiate the member variable
    setCentralWidget(scoreViewport);

    // TODO: Connect signals from core components (e.g., Score changes) to the viewport's slots
    // TODO: Pass initial score data to the viewport: scoreViewport->setScore(...);
}


// --- Slots ---

void MainWindow::openFileAction() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Score"), currentFilePath, tr("Music Trainer Files (*.mtx);;All Files (*)")); // Start in current dir
    if (!fileName.isEmpty()) {
        try {
            // Load the score using ScoreLoader
            Domain::Score loadedScore = IO::ScoreLoader::loadFromFile(fileName.toStdString());

            // Replace the current score
            currentScore = std::make_unique<Domain::Score>(std::move(loadedScore));

            // Update file path
            currentFilePath = fileName;
            setWindowTitle(QString("Music Trainer - %1").arg(QFileInfo(fileName).fileName()));

            // Clear undo history for the new score
            if(commandHistory) commandHistory->clear();
            updateUndoRedoActions(); // Update UI state

            // Update viewport
            if(scoreViewport) {
                scoreViewport->setScore(currentScore.get()); // Pass new score data
                // TODO: Reset viewport scroll/zoom?
                scoreViewport->update();
            }

            // Trigger initial validation
            validateScoreAndUpdateFeedback();

            statusBar()->showMessage(tr("Opened %1").arg(QFileInfo(fileName).fileName()), 5000);

        } catch (const std::exception& e) {
             std::cerr << "Error loading file " << fileName.toStdString() << ": " << e.what() << std::endl;
             QMessageBox::critical(this, tr("Load Error"), tr("Could not load file '%1':\n%2").arg(fileName).arg(e.what()));
             statusBar()->showMessage(tr("Error loading file"), 5000);
        }
    }
}

// Helper function for saving, called by saveFileAction and potentially others
bool MainWindow::saveScoreToFile(const QString& filePath) {
     if (!currentScore) {
         QMessageBox::warning(this, tr("Save Error"), tr("No score data to save."));
         return false;
     }
     try {
         IO::ScoreLoader::saveToFile(*currentScore, filePath.toStdString());
         currentFilePath = filePath; // Update current path after successful save
         setWindowTitle(QString("Music Trainer - %1").arg(QFileInfo(filePath).fileName()));
         statusBar()->showMessage(tr("Saved to %1").arg(QFileInfo(filePath).fileName()), 5000);
         // TODO: Mark document as 'not modified'
         return true;
     } catch (const std::exception& e) {
         std::cerr << "Error saving file " << filePath.toStdString() << ": " << e.what() << std::endl;
         QMessageBox::critical(this, tr("Save Error"), tr("Could not save file '%1':\n%2").arg(filePath).arg(e.what()));
         statusBar()->showMessage(tr("Error saving file"), 5000);
         return false;
     }
}

bool MainWindow::saveFileAsAction() {
     QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Score As"), currentFilePath, tr("Music Trainer Files (*.mtx);;All Files (*)"));
     if (fileName.isEmpty()) {
         return false; // User cancelled
     }
     // Ensure extension (optional)
     // if (!fileName.endsWith(".mtx")) fileName += ".mtx";
     return saveScoreToFile(fileName);
}


void MainWindow::saveFileAction() {
     if (currentFilePath.isEmpty()) {
         // If no file path is set, behave like "Save As..."
         saveFileAsAction();
     } else {
         // Save to the current file path
         saveScoreToFile(currentFilePath);
     }
}

// --- Public Methods ---

void MainWindow::requestNoteAddition(const Domain::Note& note) {
    if (currentScore && commandHistory) {
        // Create and execute the command via the history manager
        auto command = std::make_unique<Commands::AddNoteCommand>(*currentScore, note);
        commandHistory->executeCommand(std::move(command));

        // Update UI after command execution (Undo/Redo will also call this)
        validateScoreAndUpdateFeedback();
        if (scoreViewport) {
            scoreViewport->update();
        }
        updateUndoRedoActions(); // Update action states
    }
}

void MainWindow::requestSelectedNotesDeletion(const std::vector<const Domain::Note*>& notesToDelete) {
    if (currentScore && commandHistory && !notesToDelete.empty()) {
        // Create a vector of note *values* to store in the command
        std::vector<Domain::Note> noteValues;
        noteValues.reserve(notesToDelete.size());
        for(const Domain::Note* notePtr : notesToDelete) {
            if (notePtr) {
                noteValues.push_back(*notePtr); // Copy the note value
            }
        }

        if (!noteValues.empty()) {
            // Create and execute the command
            auto command = std::make_unique<Commands::DeleteNoteCommand>(*currentScore, std::move(noteValues));
            commandHistory->executeCommand(std::move(command));

void MainWindow::requestNotesMove(const std::vector<Domain::Note>& originalNotes, double timeOffsetBeats, int pitchOffsetSemitones) {
    if (currentScore && commandHistory && !originalNotes.empty()) {
        // Create and execute the command
        // The command constructor calculates the new positions/pitches
        auto command = std::make_unique<Commands::MoveNotesCommand>(
            *currentScore,
            originalNotes, // Pass copies of original notes
            timeOffsetBeats,
            pitchOffsetSemitones
        );
        commandHistory->executeCommand(std::move(command));

        // Update UI after command execution
        validateScoreAndUpdateFeedback();
        if (scoreViewport) {
            scoreViewport->update();
        }
        updateUndoRedoActions();
    }

void MainWindow::requestNoteResize(const Domain::Note& originalNote, double newDuration) {
    if (currentScore && commandHistory) {
        try {
            // Create and execute the command
            auto command = std::make_unique<Commands::ResizeNoteCommand>(
                *currentScore,
                originalNote, // Pass copy of original note
                newDuration
            );
            commandHistory->executeCommand(std::move(command));

            // Update UI after command execution
            validateScoreAndUpdateFeedback();
            if (scoreViewport) {
                scoreViewport->update();
            }
            updateUndoRedoActions(); // Update action states
        } catch (const std::exception& e) {
            // Catch errors from command constructor (e.g., invalid duration)
            std::cerr << "MainWindow: Error creating resize command: " << e.what() << std::endl;
            QMessageBox::warning(this, "Resize Note Error", QString("Could not resize note: %1").arg(e.what()));
        }
    }
}


void MainWindow::requestNoteResize(const Domain::Note& originalNote, double newDuration) {
    if (currentScore && commandHistory) {
        try {
            // Create and execute the command
            auto command = std::make_unique<Commands::ResizeNoteCommand>(
                *currentScore,
                originalNote, // Pass copy of original note
                newDuration
            );
            commandHistory->executeCommand(std::move(command));

            // Update UI after command execution
            validateScoreAndUpdateFeedback();
            if (scoreViewport) {
                scoreViewport->update();
            }
            updateUndoRedoActions();
        } catch (const std::exception& e) {
            // Catch errors from command constructor (e.g., invalid duration)
            std::cerr << "MainWindow: Error creating resize command: " << e.what() << std::endl;
            QMessageBox::warning(this, "Resize Note Error", QString("Could not resize note: %1").arg(e.what()));
        }
    }
}

}


            // Update UI after command execution
            validateScoreAndUpdateFeedback();
            if (scoreViewport) {
                scoreViewport->update();
            }
            // Update undo/redo action states
            updateUndoRedoActions();

void MainWindow::onTempoChanged(int bpm) {
    if (bpm >= 30 && bpm <= 240) { // Validate range just in case
        currentTempoBPM = static_cast<double>(bpm);
        std::cout << "MainWindow: Tempo changed to " << currentTempoBPM << " BPM." << std::endl; // Debug
        // TODO: If playback timer is running, update its interval
        // if (isPlaying && playbackTimer) {
        //     double beatsPerSecond = currentTempoBPM / 60.0;
        //     double timerIntervalMs = (1.0 / beatsPerSecond / 4.0) * 1000.0; // Example: Update 4 times per beat
        //     playbackTimer->setInterval(static_cast<int>(timerIntervalMs));
        // }
    }
}

        }
    }
}


// --- Private Slots / Methods ---

void MainWindow::validateScoreAndUpdateFeedback() {
    if (!ruleEngine || !currentScore || !feedbackLabelPtr) {
        return; // Not ready yet
    }

    std::cout << "MainWindow: Validating score..." << std::endl; // Debug
    Rules::ValidationResult result = ruleEngine->validateScore(*currentScore);

    QString feedbackText;
    if (result.violations.empty()) {
        feedbackText = "No rule violations detected.";
    } else {
        feedbackText = QString("Violations (%1):\n").arg(result.violations.size());
        int count = 0;
        for (const auto& violation : result.violations) {
            feedbackText += QString("- [%1] %2 (Severity: %3)\n")
                                .arg(QString::fromStdString(violation.ruleId))
                                .arg(QString::fromStdString(violation.description))
                                .arg(violation.severity == Rules::Severity::Error ? "Error" : "Warning");
            // TODO: Add location info (e.g., beat number)
            // TODO: Add involved notes info
            if (++count >= 10) { // Limit displayed violations for brevity
                 feedbackText += "... (more violations exist)";
                 break;
            }
        }
    }

    feedbackLabelPtr->setText(feedbackText);
     std::cout << "MainWindow: Feedback updated." << std::endl; // Debug
}

void MainWindow::handleMidiEvent(const Midi::MidiEvent& event) {
    // This function will be called by the MidiSystem's callback mechanism
    // (potentially on a different thread depending on the MidiSystem implementation)

    if (event.isNoteOn()) {
        std::cout << "MIDI Note On Received: Note=" << static_cast<int>(event.data1)
                  << ", Vel=" << static_cast<int>(event.data2)
                  << ", Chan=" << static_cast<int>(event.getChannel()) << std::endl;

        // Create a domain Note object from the MIDI event
        try {
            Domain::Pitch pitch(event.data1); // data1 is MIDI note number
            int velocity = event.data2;

            // TODO: Determine the correct start Position based on application state
            // (e.g., current playback cursor, step input position). Using beat 0 for now.
            Domain::Position startTime(0.0);

            // TODO: Determine correct duration (e.g., fixed step duration, or wait for Note Off)
            double duration = 1.0; // Default 1 beat

            // TODO: Determine target voice based on MIDI channel or UI selection
            int targetVoiceId = 0; // Default voice 0

            Domain::Note midiNote(pitch, startTime, duration, velocity, targetVoiceId);

            // Request the note addition (which will handle validation, score update, UI refresh)
            // NOTE: If this callback is on a different thread than the main UI thread,
            // calling requestNoteAddition directly is unsafe because it modifies UI elements (feedbackLabelPtr)
            // and interacts with non-thread-safe objects (Score, RuleEngine maybe).
            // A mechanism like Qt::QueuedConnection or posting an event to the main thread's event loop
            // would be required for thread safety. For now, we assume single-threaded for simplicity.
             requestNoteAddition(midiNote);

        } catch (const std::exception& e) {
            std::cerr << "Error processing MIDI Note On event: " << e.what() << std::endl;
        }
    } else if (event.isNoteOff()) {
         std::cout << "MIDI Note Off Received: Note=" << static_cast<int>(event.data1)
                   << ", Vel=" << static_cast<int>(event.data2)
                   << ", Chan=" << static_cast<int>(event.getChannel()) << std::endl;
         // TODO: Implement logic to handle Note Off, e.g., adjust duration of previously started note
    }
    // Handle other MIDI event types (CC, Pitch Bend, etc.) if needed

void MainWindow::undo() {
    if (commandHistory && commandHistory->canUndo()) {
        std::cout << "MainWindow: Undoing command: " << commandHistory->getUndoText() << std::endl; // Debug
        commandHistory->undo(); // Command's undo method modifies the score
        // Update UI after undo
        validateScoreAndUpdateFeedback();
        if (scoreViewport) {
            scoreViewport->update();
        }
        // Update enabled state of undo/redo actions
        updateUndoRedoActions();
    }
}

void MainWindow::redo() {
    if (commandHistory && commandHistory->canRedo()) {
        std::cout << "MainWindow: Redoing command: " << commandHistory->getRedoText() << std::endl; // Debug
        commandHistory->redo(); // Command's execute method modifies the score
        // Update UI after redo
        validateScoreAndUpdateFeedback();
        if (scoreViewport) {
            scoreViewport->update();
        }
        // Update enabled state of undo/redo actions
        updateUndoRedoActions();
    }
}

}

void MainWindow::createToolBars() {
    // Transport Toolbar
    QToolBar *transportToolBar = addToolBar(tr("Transport"));
    transportToolBar->setObjectName("TransportToolBar");
    transportToolBar->addAction(playAct); // Add Play action
    transportToolBar->addAction(stopAct); // Add Stop action

    // Tempo Toolbar
    QToolBar *tempoToolBar = addToolBar(tr("Tempo"));
    tempoToolBar->setObjectName("TempoToolBar");

    QLabel *tempoLabel = new QLabel(tr(" Tempo (BPM): "));
    tempoSpinBox = new QSpinBox;
    tempoSpinBox->setRange(30, 240); // Reasonable BPM range
    tempoSpinBox->setValue(static_cast<int>(currentTempoBPM));
    tempoSpinBox->setToolTip(tr("Set the playback tempo in Beats Per Minute"));
    // Connect valueChanged signal to onTempoChanged slot
    connect(tempoSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onTempoChanged);

    tempoToolBar->addWidget(tempoLabel);
    tempoToolBar->addWidget(tempoSpinBox);

}

// --- Playback Slots ---

void MainWindow::play() {
    if (isPlaying || !currentScore || !midiSystem || !midiSystem->isOutputDeviceOpen() || !playbackTimer) {
        // TODO: Maybe select/open MIDI output if not open?
        if (!midiSystem || !midiSystem->isOutputDeviceOpen()) {
             QMessageBox::warning(this, "Playback Error", "No MIDI output device open.");
             // Try opening the first output device?
             auto outputs = midiSystem->listOutputDevices();
             if (!outputs.empty()) {
                 if (!midiSystem->openOutputDevice(outputs[0])) {
                     return; // Still failed
                 }
                  statusBar()->showMessage(QString("Opened MIDI Output: %1").arg(QString::fromStdString(outputs[0])), 5000);
             } else {
                 statusBar()->showMessage("No MIDI Output devices found.", 5000);
                 return; // No output device available
             }
        } else {
             return; // Already playing or no score etc.
        }
    }

    std::cout << "MainWindow: Playback started." << std::endl;
    isPlaying = true;
    playbackPosition = Position(0.0); // Start from beginning
    // TODO: Clear any currently playing notes visually/audibly? Send AllNotesOff?
    // activePlaybackNotes.clear(); // Need member: std::set<std::pair<int, int>> activePlaybackNotes; // {voiceId, midiNote}

    // Calculate timer interval based on tempo and desired resolution
    // Example: Update 4 times per beat (16th note resolution)
    double beatsPerSecond = currentTempoBPM / 60.0;
    double updatesPerBeat = 4.0; // Resolution (e.g., 4 = 16th notes)
    double timerIntervalMs = (1.0 / (beatsPerSecond * updatesPerBeat)) * 1000.0;
    playbackTimer->setInterval(std::max(10, static_cast<int>(timerIntervalMs))); // Ensure minimum interval (e.g., 10ms)

    playbackTimer->start();
    // Update UI state (disable play, enable stop)
    playAct->setEnabled(false);
    stopAct->setEnabled(true);
    // Update viewport immediately to show cursor at start
    if (scoreViewport) scoreViewport->setPlaybackPosition(playbackPosition);
}

void MainWindow::stop() {
    if (!isPlaying || !playbackTimer) {
        return;
    }
    std::cout << "MainWindow: Playback stopped." << std::endl;
    playbackTimer->stop();
    isPlaying = false;

    // Send Note Off for any notes still considered active
    // TODO: Requires tracking active notes (e.g., in a std::set)
    // for (const auto& notePair : activePlaybackNotes) {
    //     int voiceId = notePair.first;
    //     int midiNote = notePair.second;
    //     // Assuming channel = voiceId for now
    //     if (midiSystem) midiSystem->sendNoteOff(voiceId, midiNote);
    // }
    // activePlaybackNotes.clear();

    // Send All Sound Off / All Notes Off CC messages as a fallback
    if (midiSystem) {
        for (int ch = 0; ch < 16; ++ch) {
             midiSystem->sendControlChange(ch, 120, 0); // All Sound Off
             midiSystem->sendControlChange(ch, 123, 0); // All Notes Off
        }
    }


    // Update UI state (enable play, disable stop)
    playAct->setEnabled(true);
    stopAct->setEnabled(false);
    // Update viewport to hide cursor
    if (scoreViewport) scoreViewport->setPlaybackPosition(std::nullopt);
}

void MainWindow::advancePlayback() {
    if (!isPlaying || !currentScore || !midiSystem || !midiSystem->isOutputDeviceOpen() || !playbackTimer) {
        stop(); // Stop if state is inconsistent
        return;
    }

    // Calculate time slice for this timer interval
    double beatsPerSecond = currentTempoBPM / 60.0;
    double timeSliceBeats = beatsPerSecond * (playbackTimer->interval() / 1000.0);
    Position nextPosition = playbackPosition + timeSliceBeats;

    // Find notes starting in this time slice [playbackPosition, nextPosition)
    // Find notes ending exactly at playbackPosition
    // This requires efficient querying of the Score object (TODO in Score.h)
    // For now, simple iteration (inefficient):
    std::vector<const Domain::Note*> notesToStart;
    std::vector<const Domain::Note*> notesToEnd; // Notes ending *before* nextPosition

    for (const auto& voice : currentScore->getAllVoices()) {
        for (const auto& note : voice.notes) {
            // Check for notes starting within the slice
            if (note.getStartTime() >= playbackPosition && note.getStartTime() < nextPosition) {
                notesToStart.push_back(&note);
            }
            // Check for notes ending within the slice (or exactly at start)
            // Need tolerance for end time comparison
            Position noteEndTime = note.getEndTime();
            if (noteEndTime > playbackPosition - 1e-9 && noteEndTime <= nextPosition + 1e-9) {
                 // Check if this note was considered 'active'
                 // TODO: Requires tracking active notes
                 notesToEnd.push_back(&note);
            }
        }
    }

    // Send Note Off messages for notes ending now
    for (const auto* note : notesToEnd) {
        // TODO: Check if it's in activePlaybackNotes and remove it
        // Assuming channel = voiceId for now
        midiSystem->sendNoteOff(note->getVoiceId(), note->getPitch().getMidiNoteNumber());
         std::cout << "  Sent Note Off: V" << note->getVoiceId() << " P" << (int)note->getPitch().getMidiNoteNumber() << std::endl; // Debug
    }

    // Send Note On messages for notes starting now
    for (const auto* note : notesToStart) {
        // TODO: Add to activePlaybackNotes
        // Assuming channel = voiceId for now
        midiSystem->sendNoteOn(note->getVoiceId(), note->getPitch().getMidiNoteNumber(), note->getVelocity());
         std::cout << "  Sent Note On: V" << note->getVoiceId() << " P" << (int)note->getPitch().getMidiNoteNumber() << std::endl; // Debug
    }


    // Advance playback position
    playbackPosition = nextPosition;

    // Update viewport cursor
    if (scoreViewport) scoreViewport->setPlaybackPosition(playbackPosition);

    // Check if playback reached the end of the score (TODO: Need score duration)
    // double scoreDuration = currentScore->getDuration(); // Needs implementation
    // if (playbackPosition.beats >= scoreDuration) {
    //     stop();
    // }
}


// --- Helper Methods ---

void MainWindow::updateUndoRedoActions() {
    if (commandHistory) {
        undoAct->setEnabled(commandHistory->canUndo());
        redoAct->setEnabled(commandHistory->canRedo());
        // Optionally update action text with command description
        // undoAct->setText(tr("&Undo %1").arg(QString::fromStdString(commandHistory->getUndoText())));
        // redoAct->setText(tr("&Redo %1").arg(QString::fromStdString(commandHistory->getRedoText())));
    } else {
        undoAct->setEnabled(false);
        redoAct->setEnabled(false);
    }
}



// --- Undo/Redo Slots ---

void MainWindow::undo() {
    if (commandHistory && commandHistory->canUndo()) {
        std::cout << "MainWindow: Undoing command: " << commandHistory->getUndoText() << std::endl; // Debug
        commandHistory->undo(); // Command's undo method modifies the score
        // Update UI after undo
        validateScoreAndUpdateFeedback();
        if (scoreViewport) {
            scoreViewport->update();
        }
        updateUndoRedoActions();
    }
}

void MainWindow::redo() {
    if (commandHistory && commandHistory->canRedo()) {
        std::cout << "MainWindow: Redoing command: " << commandHistory->getRedoText() << std::endl; // Debug
        commandHistory->redo(); // Command's execute method modifies the score
        // Update UI after redo
        validateScoreAndUpdateFeedback();
        if (scoreViewport) {
            scoreViewport->update();
        }
        updateUndoRedoActions();
    }
}

// --- Helper Methods ---

void MainWindow::updateUndoRedoActions() {
    if (commandHistory) {
        undoAct->setEnabled(commandHistory->canUndo());
        redoAct->setEnabled(commandHistory->canRedo());
        // Optionally update action text with command description
        // undoAct->setText(tr("&Undo %1").arg(QString::fromStdString(commandHistory->getUndoText())));
        // redoAct->setText(tr("&Redo %1").arg(QString::fromStdString(commandHistory->getRedoText())));
    } else {
        undoAct->setEnabled(false);
        redoAct->setEnabled(false);
    }
}

} // namespace UI
} // namespace MusicTrainer