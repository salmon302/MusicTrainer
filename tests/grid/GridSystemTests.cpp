#include <gtest/gtest.h>
#include "presentation/grid/DefaultGridModel.h"
#include "presentation/grid/SpatialIndex.h"
#include "presentation/grid/CoordinateTransformer.h"
#include "presentation/grid/ViewportLayoutManager.h"
#include "presentation/grid/AdaptiveGridExpansionManager.h"
#include "presentation/grid/GridMemoryManager.h"
#include "presentation/grid/GridManager.h"
#include <QApplication>
#include <QGraphicsScene>
#include <thread>
#include <chrono>

using namespace MusicTrainer::presentation::grid;
using namespace MusicTrainer::music;

// Global QApplication instance for the entire test suite
static int argc = 1;
static char* argv[] = {(char*)"dummy"};
static QApplication app(argc, argv);

class GridSystemTests : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<DefaultGridModel>();
        transformer = std::make_unique<CoordinateTransformer>();
        layoutManager = std::make_unique<ViewportLayoutManager>(*model, *transformer);
        expansionManager = std::make_unique<AdaptiveGridExpansionManager>(*model, *layoutManager);
    }

    std::unique_ptr<DefaultGridModel> model;
    std::unique_ptr<CoordinateTransformer> transformer;
    std::unique_ptr<ViewportLayoutManager> layoutManager;
    std::unique_ptr<AdaptiveGridExpansionManager> expansionManager;
};

TEST_F(GridSystemTests, ModelBasicOperations) {
    // Test setting and getting cell values
    GridCoordinate coord{5, 60};
    GridCellValue value = 42;
    
    model->setCellValue(coord, value);
    EXPECT_TRUE(model->hasCell(coord));
    EXPECT_EQ(std::get<int>(model->getCellValue(coord)), 42);

    // Test cell removal
    model->removeCell(coord);
    EXPECT_FALSE(model->hasCell(coord));
}

TEST_F(GridSystemTests, ModelRegionOperations) {
    // Set up a test region with multiple cells
    GridRegion region{0, 10, 50, 70};
    
    for (int pos = region.startPosition; pos < region.endPosition; ++pos) {
        for (int pitch = region.minPitch; pitch < region.maxPitch; pitch += 2) {
            model->setCellValue({pos, pitch}, pos * pitch);
        }
    }

    // Test region query
    auto cells = model->getCellsInRegion(region);
    EXPECT_FALSE(cells.empty());
    
    // Verify cell contents
    for (const auto& cell : cells) {
        EXPECT_GE(cell.coord.position, region.startPosition);
        EXPECT_LT(cell.coord.position, region.endPosition);
        EXPECT_GE(cell.coord.pitch, region.minPitch);
        EXPECT_LT(cell.coord.pitch, region.maxPitch);
    }
}

TEST_F(GridSystemTests, CoordinateTransformation) {
    CoordinateTransformer::TransformConfig config;
    config.cellWidth = 50.0f;
    config.cellHeight = 20.0f;
    transformer->setConfig(config);

    // Test screen to grid conversion
    float screenX = 75.0f;
    float screenY = 30.0f;
    auto gridCoord = transformer->screenToGrid(screenX, screenY);
    
    // Should be cell (1, 1) given our cell dimensions
    EXPECT_EQ(gridCoord.position, 1);
    EXPECT_EQ(gridCoord.pitch, 1);

    // Test grid to screen conversion
    auto [screenPosX, screenPosY] = transformer->gridToScreen(gridCoord);
    EXPECT_NEAR(screenPosX, 50.0f, 0.01f);
    EXPECT_NEAR(screenPosY, 20.0f, 0.01f);
}

TEST_F(GridSystemTests, ViewportManagement) {
    Rectangle viewport{0, 0, 800, 600};
    layoutManager->updateViewport(viewport);
    
    // Test viewport expansion triggers
    EXPECT_TRUE(layoutManager->needsExpansion());
    
    // Test expansion
    layoutManager->checkAndExpandViewport();
    auto newViewport = layoutManager->getCurrentViewport();
    
    EXPECT_EQ(newViewport.x, viewport.x);
    EXPECT_EQ(newViewport.y, viewport.y);
    EXPECT_EQ(newViewport.width, viewport.width);
    EXPECT_EQ(newViewport.height, viewport.height);
}

TEST_F(GridSystemTests, AdaptiveExpansion) {
    // Configure expansion manager
    AdaptiveGridExpansionManager::ExpansionConfig config;
    config.baseExpansionRate = 1.2f;
    config.cooldownPeriod = std::chrono::seconds(1);
    expansionManager->setConfig(config);

    // Test expansion pattern
    Rectangle viewport{0, 0, 800, 600};
    expansionManager->updateUsagePattern(viewport);
    
    // Should not expand immediately due to cooldown
    EXPECT_TRUE(expansionManager->isInCooldown());
    
    // Wait for cooldown
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_FALSE(expansionManager->isInCooldown());
    
    // Test expansion rate calculation
    expansionManager->checkAndExpand();
    EXPECT_NEAR(expansionManager->getCurrentExpansionRate(), config.baseExpansionRate, 0.01f);
}

TEST_F(GridSystemTests, MemoryManagement) {
    SpatialIndex index;
    GridMemoryManager manager(index);

    GridMemoryManager::MemoryConfig config;
    config.maxMemoryUsage = 1024 * 1024; // 1MB limit
    config.cleanupInterval = 100;
    config.bufferZoneSize = 1.5f;
    
    manager.setMemoryConfig(config);
    EXPECT_EQ(manager.getMemoryConfig().maxMemoryUsage, config.maxMemoryUsage);
    
    // Test active region setting
    GridRegion region{0, 100, 48, 72};
    manager.setActiveRegion(region);
    
    // Test operation reporting
    for (int i = 0; i < config.cleanupInterval + 1; ++i) {
        manager.reportOperation();
    }
}

TEST_F(GridSystemTests, GridManagerMemoryIntegration) {
    auto scene = std::make_unique<QGraphicsScene>();
    GridManager gridManager(scene.get());
    
    // Configure memory management
    GridMemoryManager::MemoryConfig config;
    config.maxMemoryUsage = 1024 * 1024;
    config.cleanupInterval = 50;
    gridManager.configureMemoryManagement(config);
    
    // Test memory tracking during operations
    auto pitch = Pitch::fromMidiNote(60); // Middle C
    Note testNote(pitch, 1.0, 0); // Create note with pitch, duration 1.0, position 0
    
    size_t initialUsage = gridManager.getCurrentMemoryUsage();
    
    // Add some notes to increase memory usage
    for (int i = 0; i < 100; ++i) {
        gridManager.addNote(testNote, i);
    }
    
    size_t finalUsage = gridManager.getCurrentMemoryUsage();
    EXPECT_GT(finalUsage, initialUsage);
    
    // Test viewport update triggers memory management
    gridManager.setViewportSize(800, 600);
    gridManager.setViewportPosition(100.0f, 50.0f);
}