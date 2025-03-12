#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QMainWindow>
#include <memory>

#include "presentation/ExercisePanel.h"
#include "presentation/TransportControls.h"
#include "presentation/FeedbackArea.h"
#include "adapters/RtMidiAdapter.h"
#include "domain/events/EventBus.h"
#include "domain/events/GuiStateEvent.h"

using namespace MusicTrainer::presentation;

class SimpleTestWindow : public QMainWindow {
public:
    SimpleTestWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        m_centralWidget = new QWidget(this);
        setCentralWidget(m_centralWidget);
        
        m_layout = new QVBoxLayout(m_centralWidget);
        
        // Create the MIDI adapter
        auto rtMidiAdapter = music::adapters::RtMidiAdapter::create();
        // Create a shared_ptr that is compatible with MusicTrainer::ports::MidiAdapter interface
        auto midiAdapter = std::shared_ptr<MusicTrainer::ports::MidiAdapter>(rtMidiAdapter.release());
        
        // Create event bus
        auto eventBus = MusicTrainer::music::events::EventBus::create();
        eventBus->start();
        
        // Initialize transport controls with MIDI adapter and event bus
        m_transportControls = new TransportControls(midiAdapter, eventBus, this);
        m_layout->addWidget(m_transportControls);
        
        resize(800, 600);
        
        setWindowTitle("MusicTrainer GUI Test");
        resize(1024, 768);
        
        // Create main layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Create header with title
        QLabel* titleLabel = new QLabel("Music Trainer - GUI Test", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        
        // Add title to main layout
        mainLayout->addWidget(titleLabel);
        
        // Create horizontal layout for components
        QHBoxLayout* componentLayout = new QHBoxLayout();
        
        // Create exercise panel and add to left side
        m_exercisePanel = new ExercisePanel(this);
        componentLayout->addWidget(m_exercisePanel, 1);
        
        // Create right side layout for transport and feedback
        QVBoxLayout* rightLayout = new QVBoxLayout();
        
        // Create and add transport controls to right top
        rightLayout->addWidget(m_transportControls);
        
        // Create and add feedback area to right bottom
        m_feedbackArea = new FeedbackArea(this);
        rightLayout->addWidget(m_feedbackArea, 1);
        
        // Add right layout to component layout
        componentLayout->addLayout(rightLayout, 2);
        
        // Add component layout to main layout
        mainLayout->addLayout(componentLayout);
        
        // Add test controls at the bottom
        QHBoxLayout* testControlsLayout = new QHBoxLayout();
        
        QPushButton* testExerciseButton = new QPushButton("Test Exercise Panel", this);
        connect(testExerciseButton, &QPushButton::clicked, this, &SimpleTestWindow::testExercisePanel);
        
        QPushButton* testFeedbackButton = new QPushButton("Test Feedback Area", this);
        connect(testFeedbackButton, &QPushButton::clicked, this, &SimpleTestWindow::testFeedbackArea);
        
        testControlsLayout->addWidget(testExerciseButton);
        testControlsLayout->addWidget(testFeedbackButton);
        
        mainLayout->addLayout(testControlsLayout);
        
        // Set main layout
        m_centralWidget->setLayout(mainLayout);
    }

    ~SimpleTestWindow() = default;

private slots:
    void testExercisePanel() {
        // Set example exercise text
        m_exercisePanel->setExerciseDescription(
            "<h2>Two-Part Counterpoint Exercise</h2>"
            "<p>Complete the second voice to create valid two-part counterpoint "
            "following these rules:</p>"
            "<ul>"
            "<li>Avoid parallel fifths and octaves</li>"
            "<li>Prefer contrary motion between voices</li>"
            "<li>End on a perfect consonance</li>"
            "</ul>"
        );
        
        // Set progress
        m_exercisePanel->setProgress(35);
        
        // Add sample rule violations
        m_exercisePanel->clearRuleViolations();
        m_exercisePanel->addRuleViolation("Parallel fifth found between measure 2-3", nullptr, 2);
        m_exercisePanel->addRuleViolation("Consider using contrary motion here", nullptr, 1);
        
        QMessageBox::information(this, "Test", "Exercise panel updated with test data");
    }
    
    void testFeedbackArea() {
        // Add validation messages
        m_feedbackArea->clearFeedback();
        m_feedbackArea->addValidationMessage("Parallel fifth detected between soprano and bass", true);
        m_feedbackArea->addValidationMessage("Consider using contrary motion in measure 3", false);
        
        QMessageBox::information(this, "Test", "Feedback area updated with test data");
    }

private:
    ExercisePanel* m_exercisePanel;
    TransportControls* m_transportControls;
    FeedbackArea* m_feedbackArea;
    QWidget* m_centralWidget;
    QVBoxLayout* m_layout;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    SimpleTestWindow window;
    window.show();
    return app.exec();
}