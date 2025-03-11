#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>
#include <iomanip>
#include <QApplication>
#include <QGraphicsScene>
#include <QPointF>
#include <QRectF>

// Old implementation
#include "presentation/NoteGrid.h"
#include "presentation/ViewportManager.h"

// New implementation
#include "presentation/grid/GridManager.h"
#include "presentation/grid/NoteGridAdapter.h"
#include "presentation/grid/ViewportManagerAdapter.h"
#include "presentation/grid/DefaultGridModel.h"
#include "presentation/grid/SpatialIndex.h"
#include "presentation/grid/BatchedGridRenderer.h"
#include "presentation/grid/QtBatchedGridRenderer.h"

using namespace MusicTrainer::presentation;
using namespace MusicTrainer::presentation::grid;
using namespace MusicTrainer::music;

// Helper class for timing operations
class Timer {
public:
    Timer() : m_start(std::chrono::high_resolution_clock::now()) {}
    
    double elapsedMs() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - m_start).count();
    }
    
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};

// Test fixture for performance benchmarks
class GridPerformanceBenchmark : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application for tests
        static int argc = 1;
        static char* argv[] = { const_cast<char*>("GridPerformanceBenchmark") };
        static QApplication app(argc, argv);
        
        // Create a scene for testing
        m_scene = std::make_unique<QGraphicsScene>();
        
        // Initialize old implementation
        m_oldNoteGrid = std::make_unique<NoteGrid>(m_scene.get());
        m_oldViewportManager = std::make_unique<ViewportManager>(m_oldNoteGrid.get());
        
        // Initialize new implementation
        m_gridManager = std::make_unique<GridManager>(m_scene.get());
        
        // Configure memory management
        GridMemoryManager::MemoryConfig config;
        config.maxMemoryUsage = 50 * 1024 * 1024; // 50MB limit
        config.cleanupInterval = 100;
        config.bufferZoneSize = 1.5f;
        m_gridManager->configureMemoryManagement(config);
        
        // Set initial viewport size
        m_gridManager->setViewportSize(800, 600);
        m_oldViewportManager->updateViewSize(QSize(800, 600));
        
        // Set up random number generator
        m_rng.seed(42); // Use fixed seed for reproducibility
    }
    
    // Helper method to generate random notes
    std::vector<Note> generateRandomNotes(int count, int minPitch = 48, int maxPitch = 84, 
                                         int minPos = 0, int maxPos = 100) {
        std::vector<Note> notes;
        notes.reserve(count);
        
        std::uniform_int_distribution<int> pitchDist(minPitch, maxPitch);
        std::uniform_int_distribution<int> posDist(minPos, maxPos);
        std::uniform_real_distribution<double> durDist(0.25, 4.0);
        
        for (int i = 0; i < count; ++i) {
            // Create a note with proper constructor
            Pitch pitch = Pitch::fromMidiNote(pitchDist(m_rng));
            double duration = durDist(m_rng);
            int position = posDist(m_rng);
            Note note(pitch, duration, position);
            notes.push_back(note);
        }
        
        return notes;
    }
    
    // Helper method to print benchmark results
    void printBenchmarkResult(const std::string& operation, double oldTime, double newTime) {
        double speedup = oldTime / newTime;
        std::cout << std::left << std::setw(30) << operation 
                  << std::right << std::setw(10) << std::fixed << std::setprecision(2) << oldTime << " ms  " 
                  << std::setw(10) << newTime << " ms  "
                  << std::setw(10) << speedup << "x" << std::endl;
    }
    
    std::unique_ptr<QGraphicsScene> m_scene;
    std::unique_ptr<NoteGrid> m_oldNoteGrid;
    std::unique_ptr<ViewportManager> m_oldViewportManager;
    std::unique_ptr<GridManager> m_gridManager;
    std::mt19937 m_rng;
};

// Test adding a large number of notes
TEST_F(GridPerformanceBenchmark, AddNotesBenchmark) {
    const int noteCount = 1000;
    auto notes = generateRandomNotes(noteCount);
    
    // Benchmark old implementation
    Timer oldTimer;
    for (int i = 0; i < noteCount; ++i) {
        m_oldNoteGrid->addNote(notes[i], 0, i % 100);
    }
    double oldTime = oldTimer.elapsedMs();
    
    // Clear old grid
    m_oldNoteGrid->clear();
    
    // Benchmark new implementation
    Timer newTimer;
    for (int i = 0; i < noteCount; ++i) {
        m_gridManager->addNote(notes[i], i % 100);
    }
    double newTime = newTimer.elapsedMs();
    
    // Print results
    printBenchmarkResult("Add " + std::to_string(noteCount) + " notes", oldTime, newTime);
    
    // Expect new implementation to be faster
    EXPECT_LT(newTime, oldTime);
}

// Test querying notes in a region
TEST_F(GridPerformanceBenchmark, QueryRegionBenchmark) {
    const int noteCount = 5000;
    auto notes = generateRandomNotes(noteCount);
    
    // Add notes to both implementations
    for (int i = 0; i < noteCount; ++i) {
        m_oldNoteGrid->addNote(notes[i], 0, i % 100);
        m_gridManager->addNote(notes[i], i % 100);
    }
    
    // Define query region
    QRectF queryRect(10, 60, 40, 12); // Position 10-50, Pitch 60-72
    GridRegion queryRegion{10, 50, 60, 72};
    
    // Benchmark old implementation
    Timer oldTimer;
    for (int i = 0; i < 100; ++i) {
        // NoteGrid doesn't have getNotesInRegion, so we'll simulate it by checking cells in the region
        int minPos = static_cast<int>(queryRect.left());
        int maxPos = static_cast<int>(queryRect.right());
        int minPitch = static_cast<int>(queryRect.top());
        int maxPitch = static_cast<int>(queryRect.bottom());
        
        // Count notes in the region
        int count = 0;
        for (int pos = minPos; pos <= maxPos; ++pos) {
            for (int pitch = minPitch; pitch <= maxPitch; ++pitch) {
                if (m_oldNoteGrid->hasNoteAt(pos, pitch)) {
                    count++;
                }
            }
        }
    }
    double oldTime = oldTimer.elapsedMs();
    
    // Benchmark new implementation
    Timer newTimer;
    for (int i = 0; i < 100; ++i) {
        auto newResults = m_gridManager->getNotesInRegion(queryRegion);
    }
    double newTime = newTimer.elapsedMs();
    
    // Print results
    printBenchmarkResult("Query region (100 times)", oldTime, newTime);
    
    // Expect new implementation to be faster
    EXPECT_LT(newTime, oldTime);
}

// Test viewport operations (scrolling, zooming)
TEST_F(GridPerformanceBenchmark, ViewportOperationsBenchmark) {
    const int noteCount = 2000;
    auto notes = generateRandomNotes(noteCount);
    
    // Add notes to both implementations
    for (int i = 0; i < noteCount; ++i) {
        m_oldNoteGrid->addNote(notes[i], 0, i % 100);
        m_gridManager->addNote(notes[i], i % 100);
    }
    
    // Benchmark old implementation viewport scrolling
    Timer oldScrollTimer;
    for (int i = 0; i < 100; ++i) {
        m_oldViewportManager->updateScrollPosition(QPointF(i % 50, i % 30));
    }
    double oldScrollTime = oldScrollTimer.elapsedMs();
    
    // Benchmark new implementation viewport scrolling
    Timer newScrollTimer;
    for (int i = 0; i < 100; ++i) {
        m_gridManager->setViewportPosition(i % 50, i % 30);
    }
    double newScrollTime = newScrollTimer.elapsedMs();
    
    // Print results
    printBenchmarkResult("Viewport scrolling (100 times)", oldScrollTime, newScrollTime);
    
    // Benchmark old implementation viewport zooming
    Timer oldZoomTimer;
    for (int i = 0; i < 20; ++i) {
        float zoom = 0.5f + (i % 10) / 10.0f;
        m_oldViewportManager->updateZoomLevel(zoom);
    }
    double oldZoomTime = oldZoomTimer.elapsedMs();
    
    // Benchmark new implementation viewport zooming
    Timer newZoomTimer;
    for (int i = 0; i < 20; ++i) {
        float zoom = 0.5f + (i % 10) / 10.0f;
        m_gridManager->setZoom(zoom);
    }
    double newZoomTime = newZoomTimer.elapsedMs();
    
    // Print results
    printBenchmarkResult("Viewport zooming (20 times)", oldZoomTime, newZoomTime);
    
    // Expect new implementation to be faster
    EXPECT_LT(newScrollTime, oldScrollTime);
    EXPECT_LT(newZoomTime, oldZoomTime);
}

// Test memory usage
TEST_F(GridPerformanceBenchmark, MemoryUsageBenchmark) {
    const int noteCount = 10000;
    auto notes = generateRandomNotes(noteCount);
    
    // Add notes to both implementations
    for (int i = 0; i < noteCount; ++i) {
        m_oldNoteGrid->addNote(notes[i], 0, i % 100);
        m_gridManager->addNote(notes[i], i % 100);
    }
    
    // Get memory usage of new implementation
    size_t newMemoryUsage = m_gridManager->getCurrentMemoryUsage();
    
    // We can't directly measure the old implementation's memory usage,
    // but we can print the new implementation's usage for reference
    std::cout << "Memory usage with " << noteCount << " notes: " 
              << (newMemoryUsage / 1024) << " KB" << std::endl;
    
    // Test memory cleanup
    m_gridManager->setViewportPosition(200, 200); // Move viewport away from notes
    size_t newMemoryUsageAfterCleanup = m_gridManager->getCurrentMemoryUsage();
    
    std::cout << "Memory usage after cleanup: " 
              << (newMemoryUsageAfterCleanup / 1024) << " KB" << std::endl;
    std::cout << "Memory reduction: " 
              << ((newMemoryUsage - newMemoryUsageAfterCleanup) * 100.0 / newMemoryUsage) 
              << "%" << std::endl;
    
    // Expect memory usage to decrease after cleanup
    EXPECT_LT(newMemoryUsageAfterCleanup, newMemoryUsage);
}

// Test large grid expansion
TEST_F(GridPerformanceBenchmark, GridExpansionBenchmark) {
    // Benchmark old implementation
    Timer oldTimer;
    for (int i = 0; i < 10; ++i) {
        m_oldViewportManager->expandGrid(MusicTrainer::presentation::GridDirection::Right, 10);
        m_oldViewportManager->expandGrid(MusicTrainer::presentation::GridDirection::Down, 12);
    }
    double oldTime = oldTimer.elapsedMs();
    
    // Benchmark new implementation
    Timer newTimer;
    for (int i = 0; i < 10; ++i) {
        m_gridManager->expandGrid(MusicTrainer::presentation::grid::GridDirection::Right, 10);
        m_gridManager->expandGrid(MusicTrainer::presentation::grid::GridDirection::Down, 12);
    }
    double newTime = newTimer.elapsedMs();
    
    // Print results
    printBenchmarkResult("Grid expansion (10 times)", oldTime, newTime);
    
    // Expect new implementation to be faster
    EXPECT_LT(newTime, oldTime);
}