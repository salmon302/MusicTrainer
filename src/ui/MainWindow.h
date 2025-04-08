#ifndef MUSIC_TRAINER_UI_MAINWINDOW_H
#define MUSIC_TRAINER_UI_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <memory> // For std::unique_ptr

// Forward declarations for UI elements
namespace MusicTrainer { namespace UI { class ScoreViewportWidget; } }
// Forward declarations for Core components
namespace MusicTrainer { namespace Domain { class Score; class Note; } }
namespace MusicTrainer { namespace Rules { class RuleEngine; class Preset; } }
namespace MusicTrainer { namespace Midi { class MidiSystem; struct MidiEvent; } }
namespace MusicTrainer { namespace Commands { class CommandHistory; } } // Added Commands


namespace MusicTrainer {
namespace UI {

class MainWindow : public QMainWindow
{
    Q_OBJECT // Required for classes using signals/slots

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Public method for viewport to request score modification
    void requestNoteAddition(const Domain::Note& note);
    void requestSelectedNotesDeletion(const std::vector<const Domain::Note*>& notesToDelete);
    void requestNotesMove(const std::vector<Domain::Note>& originalNotes, double timeOffsetBeats, int pitchOffsetSemitones);
    void requestNoteResize(const Domain::Note& originalNote, double newDuration); // Added resize request

private slots:
    // Placeholder slots for actions (e.g., File->Open)
    void openFileAction();
    void saveFileAction();
    // Add slots for playback controls, etc.

    // Slot/Method to update feedback display
    void validateScoreAndUpdateFeedback();
    // Slot to handle incoming MIDI events
    void handleMidiEvent(const Midi::MidiEvent& event);
    // Slots for Undo/Redo actions
    void undo();
    void redo();
    // Slots for Playback actions
    void play();
    void stop();
    // Slot for playback timer
    void advancePlayback();
    // Slot for tempo change
    void onTempoChanged(int bpm);
    // Slot for metronome toggle
    void toggleMetronome(bool enabled);
    // Slot for loading selected exercise
    void loadSelectedExercise();


private:
    // Setup methods
    void createActions();
    void createMenus();
    void createToolBars(); // Added toolbar creation
    void createStatusBar();
    void createDockWidgets(); // For panels like feedback, exercise info
    void setupCentralWidget(); // Where the main score view might go
    void updateUndoRedoActions(); // Helper to enable/disable actions
    // Helpers for saving
    bool saveScoreToFile(const QString& filePath);
    bool saveFileAsAction();

    // --- Member Variables ---
    // If using Qt Designer .ui file:
    // Ui::MainWindow *ui;

    // Pointers to main widgets/panels
    ScoreViewportWidget* scoreViewport; // Now using the actual widget
    // ControlPanelWidget* controlPanel;
    // FeedbackPanelWidget* feedbackPanel;
    // ExercisePanelWidget* exercisePanel;

    // Actions (for menus and toolbars)
    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *playAct;
    QAction *stopAct;
    QAction *metronomeAct; // Added Metronome action
    // Add more actions...

    // Menus
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    // Add more menus...

    // --- Core Components ---
    std::unique_ptr<Domain::Score> currentScore; // Owns the active score data
    std::unique_ptr<Rules::RuleEngine> ruleEngine; // Owns the rule engine instance
    std::unique_ptr<Midi::MidiSystem> midiSystem; // Owns the MIDI system instance
    std::unique_ptr<Commands::CommandHistory> commandHistory; // Owns the undo/redo history

    // --- UI Element Pointers (for easy access) ---
    QLabel* feedbackLabelPtr = nullptr;
    QSpinBox* tempoSpinBox = nullptr;
    QListWidget* exerciseListWidget = nullptr; // Pointer to exercise list

    // --- State ---
    QString currentFilePath; // Path to the currently open score file
    bool isPlaying = false;
    Domain::Position playbackPosition; // Current playback position in beats
    QTimer* playbackTimer = nullptr; // Timer for driving playback steps
    double currentTempoBPM = 120.0; // Current tempo
    bool isMetronomeEnabled = false; // Metronome state

}; // class MainWindow

} // namespace UI
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_UI_MAINWINDOW_H