#pragma once

#include <QMainWindow>
#include <QSettings>
#include <memory>
#include "domain/exercises/Exercise.h"
#include "domain/exercises/ExerciseRepository.h"
#include "domain/rules/ValidationPipeline.h"

// Forward declarations
class QDockWidget;

namespace MusicTrainer {
namespace domain {
namespace exercises {
    class Exercise;
    class ExerciseRepository;
}
}
namespace music {
    class Score;
    namespace rules {
        class ValidationPipeline;
    }
}
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

private Q_SLOTS:
    void onExerciseBrowserRequested();
    void onSettingsDialogRequested();
    void onAboutDialogRequested();

private:
    void setupMenus();
    void connectSignals();
    void setupDockWidgets();
    void setupStatusBar();

    std::shared_ptr<ports::MidiAdapter> m_midiAdapter;
    std::shared_ptr<MusicTrainer::music::Score> m_score; // Fully qualified namespace
    std::shared_ptr<domain::exercises::ExerciseRepository> m_exerciseRepository;
    std::shared_ptr<domain::exercises::Exercise> m_currentExercise;
    std::unique_ptr<music::rules::ValidationPipeline> m_validationPipeline;

    // UI Components
    ScoreView* m_scoreView{nullptr};
    TransportControls* m_transportControls{nullptr};
    ExercisePanel* m_exercisePanel{nullptr};
    FeedbackArea* m_feedbackArea{nullptr};

    // Dock Widgets
    QDockWidget* m_transportDock{nullptr};
    QDockWidget* m_exerciseDock{nullptr};
    QDockWidget* m_feedbackDock{nullptr};
    
    // Exercise handling
    void loadExercise(std::shared_ptr<domain::exercises::Exercise> exercise);
    void validateExercise();
};

} // namespace presentation
} // namespace MusicTrainer