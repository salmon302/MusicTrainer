#include "presentation/MainWindow.h"
#include "presentation/ScoreView.h"
#include "presentation/TransportControls.h"
#include "presentation/ExercisePanel.h"
#include "presentation/FeedbackArea.h"
#include "presentation/ExerciseBrowser.h"
#include "presentation/SettingsDialog.h"
#include "domain/adapters/InMemoryExerciseRepository.h"
#include "domain/exercises/Exercise.h"
#include "domain/rules/ValidationPipeline.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/ports/MidiAdapter.h"
#include "domain/state/SettingsState.h"
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QDebug>
#include <QStatusBar>
#include <QToolBar>
#include <QtWidgets/QApplication>
#include <QDockWidget>

namespace MusicTrainer::presentation {

MainWindow::MainWindow(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget *parent)
    : QMainWindow(parent)
    , m_midiAdapter(midiAdapter)
{
    // Initialize state management first
    initializeStateManagement();
    
    // Create central widget
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create main layout
    auto* mainLayout = new QVBoxLayout(centralWidget);
    
    // Create score view with event bus
    m_scoreView = new ScoreView(m_eventBus, this);
    mainLayout->addWidget(m_scoreView);
    
    // Create and setup dock widgets
    setupDockWidgets();
    
    // Create status bar
    setupStatusBar();

    // Create transport controls with event bus
    m_transportControls = new TransportControls(m_midiAdapter, m_eventBus, this);
    m_transportDock = new QDockWidget(tr("Transport"), this);
    m_transportDock->setWidget(m_transportControls);
    addDockWidget(Qt::TopDockWidgetArea, m_transportDock);

    // Create and connect exercise panel
    m_exercisePanel = new ExercisePanel(this);
    m_exerciseDock = new QDockWidget(tr("Exercise"), this);
    m_exerciseDock->setWidget(m_exercisePanel);
    addDockWidget(Qt::RightDockWidgetArea, m_exerciseDock);

    // Create and connect feedback area
    m_feedbackArea = new FeedbackArea(this);
    m_feedbackDock = new QDockWidget(tr("Feedback"), this);
    m_feedbackDock->setWidget(m_feedbackArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_feedbackDock);

    // Connect signals
    connectSignals();
    
    // Restore window state if available
    QSettings settings;
    if (settings.contains("mainWindowGeometry")) {
        restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    }
    if (settings.contains("mainWindowState")) {
        restoreState(settings.value("mainWindowState").toByteArray());
    }
    
    setMinimumSize(800, 600);
}

MainWindow::~MainWindow()
{
    // Save window state to settings
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());

    // Stop state management
    if (m_stateCoordinator) {
        m_stateCoordinator->stop();
    }
}

void MainWindow::setupMenus()
{
    // File menu
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    
    // Fix deprecated addAction calls with new Qt6 format
    auto* newAction = new QAction(tr("&New Exercise"), this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, [this]() {
        if (m_score && m_score->isDirty()) {
            auto result = QMessageBox::question(this, tr("Unsaved Changes"),
                tr("There are unsaved changes. Would you like to save before creating a new exercise?"),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            
            if (result == QMessageBox::Cancel) {
                return;
            }
            if (result == QMessageBox::Yes) {
                // TODO: Implement save functionality
            }
        }
        // Create a shared_ptr from the unique_ptr returned by Score::create()
        auto scorePtr = MusicTrainer::music::Score::create();
        m_score = std::shared_ptr<MusicTrainer::music::Score>(scorePtr.release());
        m_scoreView->setScore(m_score);
    });
    fileMenu->addAction(newAction);
    
    auto* openAction = new QAction(tr("&Open..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onExerciseBrowserRequested);
    fileMenu->addAction(openAction);
    
    fileMenu->addSeparator();
    
    auto* saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, [this]() {
        // TODO: Implement save functionality
    });
    fileMenu->addAction(saveAction);
    
    auto* saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, [this]() {
        // TODO: Implement save as functionality
    });
    fileMenu->addAction(saveAsAction);
    
    fileMenu->addSeparator();
    
    auto* quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(quitAction);

    // Edit menu
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    
    auto* undoAction = new QAction(tr("&Undo"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this]() {
        // TODO: Implement undo functionality
    });
    editMenu->addAction(undoAction);
    
    auto* redoAction = new QAction(tr("&Redo"), this);
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, [this]() {
        // TODO: Implement redo functionality
    });
    editMenu->addAction(redoAction);

    editMenu->addSeparator();
    
    auto* settingsAction = new QAction(tr("&Settings..."), this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsDialogRequested);
    editMenu->addAction(settingsAction);

    // View Menu
    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    if (m_transportDock) viewMenu->addAction(m_transportDock->toggleViewAction());
    if (m_exerciseDock) viewMenu->addAction(m_exerciseDock->toggleViewAction());
    if (m_feedbackDock) viewMenu->addAction(m_feedbackDock->toggleViewAction());
    
    // Help Menu
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    auto* aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutDialogRequested);
    helpMenu->addAction(aboutAction);
    
    auto* aboutQtAction = new QAction(tr("About &Qt"), this);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::connectSignals()
{
    // Connect GUI state change handlers
    m_stateCoordinator->subscribeToState<music::events::GuiStateEvent::ViewportState>(
        music::events::GuiStateEvent::StateType::VIEWPORT_CHANGE,
        [this](const auto& state) { onViewportChanged(state); }
    );

    m_stateCoordinator->subscribeToState<music::events::GuiStateEvent::ScoreDisplayState>(
        music::events::GuiStateEvent::StateType::SCORE_DISPLAY_CHANGE,
        [this](const auto& state) { onScoreDisplayChanged(state); }
    );

    m_stateCoordinator->subscribeToState<music::events::GuiStateEvent::SelectionState>(
        music::events::GuiStateEvent::StateType::SELECTION_CHANGE,
        [this](const auto& state) { onSelectionChanged(state); }
    );

    m_stateCoordinator->subscribeToState<music::events::GuiStateEvent::PlaybackState>(
        music::events::GuiStateEvent::StateType::PLAYBACK_STATE_CHANGE,
        [this](const auto& state) { onPlaybackStateChanged(state); }
    );

    m_stateCoordinator->subscribeToState<music::events::GuiStateEvent::MidiDeviceState>(
        music::events::GuiStateEvent::StateType::MIDI_DEVICE_CHANGE,
        [this](const auto& state) { onMidiDeviceChanged(state); }
    );

    // Connect exercise panel signals
    if (m_exercisePanel && m_scoreView) {
        // NOTE: Using static_cast to ensure proper signal/slot types
        connect(m_exercisePanel, static_cast<void (ExercisePanel::*)(const QString&)>(&ExercisePanel::exerciseChanged),
                this, [this](const QString& exerciseType) {
                    // Handle exercise type change
                    qDebug() << "Exercise type changed to:" << exerciseType;
                });
                
        // Connect check solution button
        connect(m_exercisePanel, &ExercisePanel::checkSolutionRequested,
                this, &MainWindow::validateExercise);
                
        // Connect hint button
        connect(m_exercisePanel, &ExercisePanel::hintRequested,
                this, [this](int level) {
                    // Handle hint request
                    qDebug() << "Hint requested at level:" << level;
                    
                    // Show a simple hint for now
                    QString hint;
                    if (m_currentExercise) {
                        switch (level) {
                            case 0: // Minimal hint
                                hint = tr("Check the rules for this exercise type.");
                                break;
                            case 1: // Medium hint
                                hint = tr("Look for parallel fifths or octaves in your counterpoint.");
                                break;
                            case 2: // Detailed hint
                                hint = tr("Make sure your melodic line has a good balance of steps and leaps, "
                                         "and that you approach perfect consonances by contrary or oblique motion.");
                                break;
                            default:
                                hint = tr("Review the exercise description for guidance.");
                                break;
                        }
                    } else {
                        hint = tr("No exercise is currently loaded.");
                    }
                    
                    QMessageBox::information(this, tr("Hint"), hint);
                });
    }

    // Connect transport control signals if available
    if (m_transportControls && m_midiAdapter) {
        // Implementation will be added in the MIDI integration phase
    }

    // Connect to settings state changes
    auto& settings = state::SettingsState::instance();
    connect(&settings, &state::SettingsState::midiSettingsChanged,
            this, &MainWindow::onMidiSettingsChanged);
    connect(&settings, &state::SettingsState::audioSettingsChanged,
            this, &MainWindow::onAudioSettingsChanged);
    connect(&settings, &state::SettingsState::uiSettingsChanged,
            this, &MainWindow::onUiSettingsChanged);
    connect(&settings, &state::SettingsState::ruleSettingsChanged,
            this, &MainWindow::onRuleSettingsChanged);
}

// Implement the state change handlers
void MainWindow::onViewportChanged(const music::events::GuiStateEvent::ViewportState& state) {
    if (m_scoreView) {
        m_scoreView->setViewportPosition(state.x, state.y);
        m_scoreView->setZoomLevel(state.zoomLevel);
    }
}

void MainWindow::onScoreDisplayChanged(const music::events::GuiStateEvent::ScoreDisplayState& state) {
    if (m_scoreView) {
        m_scoreView->setShowMeasureNumbers(state.showMeasureNumbers);
        m_scoreView->setShowKeySignature(state.showKeySignature);
        m_scoreView->setShowVoiceLabels(state.showVoiceLabels);
        m_scoreView->setFontSize(state.fontSize);
    }
}

void MainWindow::onSelectionChanged(const music::events::GuiStateEvent::SelectionState& state) {
    if (m_scoreView) {
        m_scoreView->setSelectedVoice(state.voiceIndex);
        m_scoreView->setSelectedMeasure(state.measureIndex);
        m_scoreView->setSelectedNote(state.noteIndex);
    }
}

void MainWindow::onPlaybackStateChanged(const music::events::GuiStateEvent::PlaybackState& state) {
    if (m_transportControls) {
        m_transportControls->setPlaybackState(state.isPlaying);
        m_transportControls->setRecordingState(state.isRecording);
        m_transportControls->setTempo(state.tempo);
        m_transportControls->setMetronomeEnabled(state.metronomeEnabled);
    }
}

void MainWindow::onMidiDeviceChanged(const music::events::GuiStateEvent::MidiDeviceState& state) {
    if (m_midiAdapter) {
        if (state.inputDeviceIndex >= 0) {
            m_midiAdapter->setInputDevice(state.inputDeviceIndex);
        }
        if (state.outputDeviceIndex >= 0) {
            m_midiAdapter->setOutputDevice(state.outputDeviceIndex);
        }
        m_midiAdapter->setMidiThrough(state.midiThrough);
        m_midiAdapter->setLatency(state.latency);
    }
}

void MainWindow::setupDockWidgets()
{
    // Transport Controls Dock
    m_transportDock = new QDockWidget(tr("Transport Controls"), this);
    m_transportDock->setObjectName("transportDock"); // Set object name for saving state
    m_transportDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    m_transportDock->setWidget(m_transportControls);
    addDockWidget(Qt::BottomDockWidgetArea, m_transportDock);
    
    // Exercise Panel Dock
    m_exerciseDock = new QDockWidget(tr("Exercise Panel"), this);
    m_exerciseDock->setObjectName("exerciseDock"); // Set object name for saving state
    m_exerciseDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_exerciseDock->setWidget(m_exercisePanel);
    addDockWidget(Qt::RightDockWidgetArea, m_exerciseDock);
    
    // Feedback Area Dock
    m_feedbackDock = new QDockWidget(tr("Feedback"), this);
    m_feedbackDock->setObjectName("feedbackDock"); // Set object name for saving state
    m_feedbackDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    m_feedbackDock->setWidget(m_feedbackArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_feedbackDock);
    
    // Make sure transport controls and feedback are tabbed together in the bottom area
    tabifyDockWidget(m_transportDock, m_feedbackDock);
    m_transportDock->raise(); // Make transport controls visible by default
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::onExerciseBrowserRequested()
{
    auto* browser = new ExerciseBrowser(this);
    
    // Set the exercise repository
    browser->setExerciseRepository(m_exerciseRepository);
    
    // Show the dialog as modal
    if (browser->exec() == QDialog::Accepted) {
        auto selectedExercise = browser->getSelectedExercise();
        if (selectedExercise) {
            // Load the selected exercise
            loadExercise(selectedExercise);
            
            // Update status bar
            statusBar()->showMessage(tr("Exercise loaded: %1").arg(QString::fromStdString(selectedExercise->getName())), 3000);
        }
    }
    
    // Dialog is automatically deleted when closed
}

void MainWindow::onSettingsDialogRequested()
{
    auto* settingsDialog = new SettingsDialog(m_midiAdapter, this);
    
    // Show the dialog as modal
    if (settingsDialog->exec() == QDialog::Accepted) {
        // In a complete implementation, we would apply the settings
        // For now, just update the status bar
        statusBar()->showMessage(tr("Settings saved"), 3000);
        
        // Update UI components based on settings
        // This would be more comprehensive in a complete implementation
        QSettings settings;
        
        // Update score view with UI settings
        settings.beginGroup("UI");
        bool showMeasureNumbers = settings.value("ShowMeasureNumbers", true).toBool();
        bool showKeySignature = settings.value("ShowKeySignature", true).toBool();
        bool showVoiceLabels = settings.value("ShowVoiceLabels", true).toBool();
        settings.endGroup();
        
        // Apply settings to score view
        if (m_scoreView) {
            m_scoreView->setShowMeasureNumbers(showMeasureNumbers);
            m_scoreView->setShowKeySignature(showKeySignature);
            m_scoreView->setShowVoiceLabels(showVoiceLabels);
        }
    }
    
    // Dialog is automatically deleted when closed
}

void MainWindow::onAboutDialogRequested()
{
    QMessageBox::about(this, tr("About MusicTrainer"),
                       tr("<h2>MusicTrainer</h2>"
                          "<p>Version 1.0.0</p>"
                          "<p>A music theory and composition training application.</p>"
                          "<p>Copyright © 2023</p>"));
}

void MainWindow::loadExercise(std::shared_ptr<domain::exercises::Exercise> exercise)
{
    if (!exercise) {
        return;
    }
    
    // Store the current exercise
    m_currentExercise = exercise;
    
    // Update the exercise panel with the exercise description
    m_exercisePanel->setExerciseDescription(QString::fromStdString(exercise->getDescription()));
    m_exercisePanel->setProgress(0);
    m_exercisePanel->clearRuleViolations();
    
    // Clear the feedback area
    m_feedbackArea->clearFeedback();
    
    // If the exercise has a template score, use it
    if (exercise->getTemplateScore()) {
        // Create a copy of the template score
        auto scorePtr = MusicTrainer::music::Score::create();
        m_score = std::shared_ptr<MusicTrainer::music::Score>(scorePtr.release());
        
        // TODO: Copy the template score to the new score
        
        // Update the score view
        m_scoreView->setScore(m_score);
    }
    
    // Set up the validation pipeline with the exercise rules
    m_validationPipeline = music::rules::ValidationPipeline::create();
    
    // Add all rules from the exercise to the validation pipeline
    for (const auto& rule : exercise->getRules()) {
        if (rule) {
            // Clone the rule to avoid modifying the original
            m_validationPipeline->addRule(std::unique_ptr<music::rules::Rule>(rule->clone()));
        }
    }
    
    // Compile the rules
    m_validationPipeline->compileRules();
}

void MainWindow::validateExercise()
{
    if (!m_score || !m_currentExercise || !m_validationPipeline) {
        QMessageBox::warning(this, tr("Validation Error"),
                            tr("No exercise is currently loaded."));
        return;
    }
    
    // Clear previous validation results
    m_exercisePanel->clearRuleViolations();
    m_feedbackArea->clearFeedback();
    
    // Validate the score
    bool result = m_validationPipeline->validate(*m_score);
    
    if (result) {
        // Score passed validation
        QMessageBox::information(this, tr("Validation Result"),
                                tr("Your solution is correct! All rules are satisfied."));
        
        // Update progress
        m_exercisePanel->setProgress(100);
        
        // Add a success message to the feedback area
        m_feedbackArea->addValidationMessage(tr("Your solution is correct! All rules are satisfied."), false);
    } else {
        // Score failed validation
        QMessageBox::warning(this, tr("Validation Result"),
                            tr("Your solution has rule violations. See the feedback area for details."));
        
        // Get the violations
        auto violations = m_validationPipeline->getViolations();
        
        // Add violations to the exercise panel and feedback area
        for (const auto& violation : violations) {
            // Add to exercise panel
            m_exercisePanel->addRuleViolation(QString::fromStdString(violation), nullptr, 2);
            
            // Add to feedback area
            m_feedbackArea->addValidationMessage(QString::fromStdString(violation), true);
        }
        
        // Update progress based on number of violations
        int progress = std::max(0, 100 - static_cast<int>(violations.size() * 20));
        m_exercisePanel->setProgress(progress);
    }
    
    // Get feedback from the validation pipeline
    auto feedback = m_validationPipeline->getFeedback();
    
    // Update the feedback area with the score
    if (m_score) {
        m_feedbackArea->updateWithScore(*m_score);
    }
}

void MainWindow::initializeStateManagement()
{
    m_eventBus = music::events::EventBus::create();
    m_stateCoordinator = GuiStateCoordinator::create(m_eventBus);

    // Start the event bus and state coordinator
    m_eventBus->start();
    m_stateCoordinator->start();
}

void MainWindow::loadSettings() {
    auto& settings = state::SettingsState::instance();
    
    // Apply UI settings
    if (m_scoreView) {
        m_scoreView->setShowMeasureNumbers(settings.getShowMeasureNumbers());
        m_scoreView->setShowKeySignature(settings.getShowKeySignature());
        m_scoreView->setShowVoiceLabels(settings.getShowVoiceLabels());
        m_scoreView->setFontSize(settings.getFontSize());
    }
    
    // Apply MIDI settings
    if (m_midiAdapter) {
        // Set MIDI devices
        QString inputDevice = settings.getMidiInputDevice();
        QString outputDevice = settings.getMidiOutputDevice();
        
        if (!inputDevice.isEmpty()) {
            // Find and set input device
            const auto inputs = m_midiAdapter->getAvailableInputs();
            for (size_t i = 0; i < inputs.size(); ++i) {
                if (QString::fromStdString(inputs[i]) == inputDevice) {
                    m_midiAdapter->setInputDevice(i);
                    break;
                }
            }
        }
        
        if (!outputDevice.isEmpty()) {
            // Find and set output device
            const auto outputs = m_midiAdapter->getAvailableOutputs();
            for (size_t i = 0; i < outputs.size(); ++i) {
                if (QString::fromStdString(outputs[i]) == outputDevice) {
                    m_midiAdapter->setOutputDevice(i);
                    break;
                }
            }
        }
        
        m_midiAdapter->setMidiThrough(settings.getMidiThrough());
        m_midiAdapter->setLatency(settings.getMidiLatency());
    }
    
    // Apply audio settings to transport controls
    if (m_transportControls) {
        m_transportControls->setMetronomeEnabled(settings.getMetronomeEnabled());
        m_transportControls->setMetronomeVolume(settings.getMetronomeVolume());
    }
}

void MainWindow::onMidiSettingsChanged() {
    if (!m_midiAdapter) return;
    
    auto& settings = state::SettingsState::instance();
    
    // Update MIDI adapter settings
    QString inputDevice = settings.getMidiInputDevice();
    QString outputDevice = settings.getMidiOutputDevice();
    
    if (!inputDevice.isEmpty()) {
        const auto inputs = m_midiAdapter->getAvailableInputs();
        for (size_t i = 0; i < inputs.size(); ++i) {
            if (QString::fromStdString(inputs[i]) == inputDevice) {
                m_midiAdapter->setInputDevice(i);
                break;
            }
        }
    }
    
    if (!outputDevice.isEmpty()) {
        const auto outputs = m_midiAdapter->getAvailableOutputs();
        for (size_t i = 0; i < outputs.size(); ++i) {
            if (QString::fromStdString(outputs[i]) == outputDevice) {
                m_midiAdapter->setOutputDevice(i);
                break;
            }
        }
    }
    
    m_midiAdapter->setMidiThrough(settings.getMidiThrough());
    m_midiAdapter->setLatency(settings.getMidiLatency());
}

void MainWindow::onAudioSettingsChanged() {
    if (!m_transportControls) return;
    
    auto& settings = state::SettingsState::instance();
    m_transportControls->setMetronomeEnabled(settings.getMetronomeEnabled());
    m_transportControls->setMetronomeVolume(settings.getMetronomeVolume());
}

void MainWindow::onUiSettingsChanged() {
    if (!m_scoreView) return;
    
    auto& settings = state::SettingsState::instance();
    m_scoreView->setShowMeasureNumbers(settings.getShowMeasureNumbers());
    m_scoreView->setShowKeySignature(settings.getShowKeySignature());
    m_scoreView->setShowVoiceLabels(settings.getShowVoiceLabels());
    m_scoreView->setFontSize(settings.getFontSize());
}

void MainWindow::onRuleSettingsChanged() {
    // Update the validation pipeline with new rule settings
    if (m_validationPipeline) {
        m_validationPipeline->updateRules();
    }
}

} // namespace MusicTrainer::presentation