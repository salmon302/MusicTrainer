#include "presentation/MainWindow.h"
#include "presentation/ScoreView.h"
#include "presentation/TransportControls.h"
#include "presentation/ExercisePanel.h"
#include "presentation/FeedbackArea.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/ports/MidiAdapter.h"
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QDebug>
#include <QStatusBar>
#include <QToolBar>
#include <QtWidgets/QApplication>
#include <QDockWidget>

using namespace music; // Add using directive for music namespace

namespace MusicTrainer::presentation {

MainWindow::MainWindow(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget *parent)
    : QMainWindow(parent)
    , m_midiAdapter(midiAdapter)
    , m_score(Score::create()) // Use factory method instead of make_shared
{
    setWindowTitle(tr("MusicTrainer"));
    resize(1024, 768);

    // Create central widget and layout
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Initialize score view as central widget
    m_scoreView = new ScoreView(this);
    centralWidget->setLayout(new QVBoxLayout);
    centralWidget->layout()->addWidget(m_scoreView);

    // Initialize dock widget contents before setting up dock widgets
    m_transportControls = new TransportControls(m_midiAdapter, this);
    m_exercisePanel = new ExercisePanel(this);
    m_feedbackArea = new FeedbackArea(this);

    // Set up dockable widgets after initializing their contents
    setupDockWidgets();

    // Set up menus and status bar
    setupMenus();
    setupStatusBar();

    // Pass the score to the view
    m_scoreView->setScore(m_score);

    // Create a default voice if none exists yet
    if (m_score && m_score->getVoiceCount() == 0) {
        m_score->addVoice(std::make_unique<Voice>(Voice::TimeSignature::commonTime()));
    }

    // Show warning if MIDI is not available
    if (!m_midiAdapter) {
        QMessageBox::warning(this, tr("MIDI Unavailable"),
            tr("MIDI functionality is not available. The application will run in limited mode.\n\n"
               "Please check your MIDI configuration and ensure you have ALSA or JACK installed."));
    }

    // Connect signals/slots
    connectSignals();

    // Load window state from settings
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
}

MainWindow::~MainWindow()
{
    // Save window state to settings
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
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
        m_score = Score::create();
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
    // Connect transport controls to score view
    if (m_transportControls && m_scoreView) {
        connect(m_transportControls, &TransportControls::playbackStarted,
                m_scoreView, [this]() {
                    // Handle playback start
                });
                
        connect(m_transportControls, &TransportControls::playbackStopped,
                m_scoreView, [this]() {
                    // Handle playback stop
                });
    }

    // Connect exercise panel signals
    if (m_exercisePanel && m_scoreView) {
        // NOTE: Using static_cast to ensure proper signal/slot types
        connect(m_exercisePanel, static_cast<void (ExercisePanel::*)(const QString&)>(&ExercisePanel::exerciseChanged),
                this, [this](const QString& exerciseType) {
                    // Handle exercise type change
                    qDebug() << "Exercise type changed to:" << exerciseType;
                });
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
    // TODO: Implement exercise browser
    QMessageBox::information(this, tr("Not Implemented"),
        tr("Exercise browser not yet implemented."));
}

void MainWindow::onSettingsDialogRequested()
{
    // Placeholder for settings dialog
    QMessageBox::information(this, tr("Settings"),
                             tr("Settings dialog will be implemented in a future update."));
}

void MainWindow::onAboutDialogRequested()
{
    QMessageBox::about(this, tr("About MusicTrainer"),
                       tr("<h2>MusicTrainer</h2>"
                          "<p>Version 1.0.0</p>"
                          "<p>A music theory and composition training application.</p>"
                          "<p>Copyright © 2023</p>"));
}

} // namespace MusicTrainer::presentation