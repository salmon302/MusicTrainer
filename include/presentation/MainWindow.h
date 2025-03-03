#pragma once

#include <QMainWindow>
#include <QSettings>
#include <memory>

// Forward declarations
class QDockWidget;

// Correct namespace forward declaration
namespace music {
    class Score;
}

namespace MusicTrainer {
namespace ports {
    class MidiAdapter;
}

namespace presentation {

class ScoreView;
class TransportControls;
class ExercisePanel;
class FeedbackArea;

/**
 * @brief Main window for the MusicTrainer application
 * 
 * This class serves as the top-level container for the GUI,
 * integrating all component panels and handling the main layout.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onExerciseBrowserRequested();
    void onSettingsDialogRequested();
    void onAboutDialogRequested();

private:
    void setupMenus();
    void connectSignals();
    void setupDockWidgets();
    void setupStatusBar();

    std::shared_ptr<ports::MidiAdapter> m_midiAdapter;
    std::shared_ptr<music::Score> m_score; // Fixed namespace reference

    // UI Components
    ScoreView* m_scoreView{nullptr};
    TransportControls* m_transportControls{nullptr};
    ExercisePanel* m_exercisePanel{nullptr};
    FeedbackArea* m_feedbackArea{nullptr};

    // Dock Widgets
    QDockWidget* m_transportDock{nullptr};
    QDockWidget* m_exerciseDock{nullptr};
    QDockWidget* m_feedbackDock{nullptr};
};

} // namespace presentation
} // namespace MusicTrainer