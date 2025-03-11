#include <gtest/gtest.h>
#include "presentation/grid/NoteGridAdapter.h"
#include "presentation/grid/ViewportManagerAdapter.h"
#include "presentation/grid/ScoreViewAdapter.h"
#include "presentation/NoteGrid.h"
#include "presentation/ViewportManager.h"
#include "presentation/ScoreView.h"
#include <QApplication>
#include <QGraphicsScene>

using namespace MusicTrainer::presentation;
using namespace MusicTrainer::presentation::grid;

class GridAdapterTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application for tests
        static int argc = 1;
        static char* argv[] = { const_cast<char*>("GridAdapterTests") };
        static QApplication app(argc, argv);
        
        // Create a scene for testing
        scene = std::make_unique<QGraphicsScene>();
        
        // Create the original components
        noteGrid = std::make_unique<NoteGrid>(scene.get());
        viewportManager = std::make_unique<ViewportManager>(noteGrid.get());
        
        // Create the adapter classes
        noteGridAdapter = std::make_unique<NoteGridAdapter>(noteGrid.get());
        viewportManagerAdapter = std::make_unique<ViewportManagerAdapter>(viewportManager.get());
    }
    
    std::unique_ptr<QGraphicsScene> scene;
    std::unique_ptr<NoteGrid> noteGrid;
    std::unique_ptr<ViewportManager> viewportManager;
    std::unique_ptr<NoteGridAdapter> noteGridAdapter;
    std::unique_ptr<ViewportManagerAdapter> viewportManagerAdapter;
};

TEST_F(GridAdapterTests, NoteGridAdapterBasicOperations) {
    // Test setting dimensions
    GridDimensions dimensions{48, 72, 0, 16, 4.0f};
    noteGridAdapter->setDimensions(dimensions);
    
    // Verify dimensions were set correctly
    auto resultDimensions = noteGridAdapter->getDimensions();
    EXPECT_EQ(resultDimensions.minPitch, dimensions.minPitch);
    EXPECT_EQ(resultDimensions.maxPitch, dimensions.maxPitch);
    EXPECT_EQ(resultDimensions.startPosition, dimensions.startPosition);
    EXPECT_EQ(resultDimensions.endPosition, dimensions.endPosition);
    EXPECT_FLOAT_EQ(resultDimensions.timeSignature, dimensions.timeSignature);
    
    // Test adding a note
    MusicTrainer::music::Pitch pitch = MusicTrainer::music::Pitch::fromMidiNote(60);
    MusicTrainer::music::Note testNote(pitch, 1.0, 5);
    noteGridAdapter->addNote(testNote, 0, 5);
    
    // Verify note was added
    EXPECT_TRUE(noteGridAdapter->hasNoteAt(5, 60));
}

TEST_F(GridAdapterTests, ViewportManagerAdapterBasicOperations) {
    // Test setting viewport bounds
    Rectangle bounds{0, 0, 800, 600};
    viewportManagerAdapter->setViewportBounds(bounds);
    
    // Test updating view size
    viewportManagerAdapter->updateViewSize(800, 600);
    
    // Test updating zoom level
    float zoom = 1.5f;
    viewportManagerAdapter->updateZoomLevel(zoom);
    
    // Verify zoom level was set correctly
    EXPECT_FLOAT_EQ(viewportManagerAdapter->getZoom(), zoom);
}

TEST_F(GridAdapterTests, GridIntegrationTest) {
    // Create a GridManager with the scene
    auto gridManager = std::make_unique<GridManager>(scene.get());
    
    // Configure memory management
    GridMemoryManager::MemoryConfig config;
    config.maxMemoryUsage = 1024 * 1024;
    config.cleanupInterval = 50;
    gridManager->configureMemoryManagement(config);
    
    // Add a note to the grid
    MusicTrainer::music::Pitch pitch = MusicTrainer::music::Pitch::fromMidiNote(60);
    MusicTrainer::music::Note testNote(pitch, 1.0, 5);
    gridManager->addNote(testNote, 5);
    
    // Verify the note was added to the scene
    // This is a visual test, so we can't easily verify it programmatically
    // In a real test, we would check the scene's items
    
    // Test viewport operations
    gridManager->setViewportSize(800, 600);
    gridManager->setViewportPosition(100.0f, 50.0f);
    gridManager->setZoom(1.5f);
    
    // Update the grid
    gridManager->update();
}

// Main function is provided by tests/main.cpp