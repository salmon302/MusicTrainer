#include <QApplication>
#include <QStyleFactory>
#include "presentation/MainWindow.h"
#include "presentation/QtTypes.h"
#include "utils/DebugUtils.h"
#include "domain/monitoring/PerformanceMonitor.h"
#include "adapters/RtMidiAdapter.h"
#include "domain/ports/MidiAdapter.h"
#include "domain/music/Note.h"
#include "domain/music/Voice.h"
#include "domain/music/Score.h"

using namespace MusicTrainer;

int main(int argc, char *argv[]) {
    // Create Qt application
    QApplication app(argc, argv);
    
    // Register all Qt types in a single call
    registerQtTypes();
    
    // Set application style and properties
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setApplicationName("MusicTrainer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MusicTrainer");
    app.setOrganizationDomain("musictrainer.org");
    
    // Initialize performance monitoring and start measuring app startup
    domain::monitoring::PerformanceMonitor::getInstance().reset();
    domain::monitoring::PerformanceMonitor::getInstance().startMeasurement("app_startup");
    
    // Initialize debug tracking
    MusicTrainer::Debug::LockTracker::clearHistory();
    
    // Create MIDI adapter using the factory method
    auto rtMidiAdapter = music::adapters::RtMidiAdapter::create();
    if (!rtMidiAdapter) {
        qWarning("Failed to create RtMidiAdapter");
        return 1;
    }
    
    // Convert to shared_ptr maintaining both interfaces
    auto adapter = std::shared_ptr<music::adapters::RtMidiAdapter>(rtMidiAdapter.release());
    
    // Create an alias shared_ptr that points to the MusicTrainer::ports::MidiAdapter interface
    std::shared_ptr<MusicTrainer::ports::MidiAdapter> midiAdapter(adapter, adapter.get());
    
    // Create and show main window with MIDI adapter
    MusicTrainer::presentation::MainWindow mainWindow(midiAdapter);
    mainWindow.show();
    
    // End the startup measurement
    domain::monitoring::PerformanceMonitor::getInstance().endMeasurement("app_startup");
    
    // Start the application event loop
    int result = app.exec();
    
    // Record final metrics
    double runtime = domain::monitoring::PerformanceMonitor::getInstance().getLatency("app_startup");
    domain::monitoring::PerformanceMonitor::getInstance().updateHealthMetrics(0.0, 0.0, runtime);
    
    // Dump debug information
    MusicTrainer::Debug::LockTracker::dumpLockHistory();
    
    return result;
}