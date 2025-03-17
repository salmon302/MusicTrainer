#include <gtest/gtest.h>
#include <future>
#include "domain/plugins/PluginManager.h"
#include "domain/plugins/RulePlugin.h"
#include "domain/plugins/PluginLoader.h"
#include "domain/rules/ValidationPipeline.h"
#include "domain/music/Score.h"
#include "domain/errors/ErrorHandler.h"

using namespace std::chrono_literals;

namespace {

class PluginSystemIntegrationTest : public ::testing::Test {
protected:
    using Score = MusicTrainer::music::Score;
    using Voice = MusicTrainer::music::Voice;
    using ValidationPipeline = MusicTrainer::music::rules::ValidationPipeline;
    using PluginManager = MusicTrainer::plugins::PluginManager;
    using RulePlugin = MusicTrainer::plugins::RulePlugin;
    using PluginLoader = MusicTrainer::plugins::PluginLoader;

    std::unique_ptr<ValidationPipeline> validationPipeline;
    std::unique_ptr<PluginManager> pluginManager;
    std::unique_ptr<PluginLoader> pluginLoader;

    void SetUp() override {
        validationPipeline = ValidationPipeline::create();
        pluginManager = PluginManager::create();
        pluginLoader = PluginLoader::create();
    }
};

TEST_F(PluginSystemIntegrationTest, DynamicRuleLoadingAndValidation) {
    // Test loading custom rule plugin
    auto rulePlugin = pluginLoader->loadPlugin("CustomVoiceLeadingRule");
    ASSERT_TRUE(rulePlugin != nullptr);
    
    // Register plugin with validation pipeline
    auto ruleId = pluginManager->registerPlugin(std::move(rulePlugin));
    EXPECT_GT(ruleId, 0);

    // Create test score
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    auto score = Score::create(timeSignature);
    auto voice = Voice::create(timeSignature);
    voice->addNote(60, 1.0); // Middle C
    voice->addNote(64, 1.0); // E
    score->addVoice(std::move(voice));

    // Validate with custom rule
    bool isValid = validationPipeline->validate(*score);
    EXPECT_TRUE(isValid);

    // Check that plugin rule was executed
    const auto& metrics = validationPipeline->getMetrics();
    EXPECT_GT(metrics.ruleExecutions, 0);
}

TEST_F(PluginSystemIntegrationTest, PluginLifecycleManagement) {
    // Test plugin loading
    auto plugin1 = pluginLoader->loadPlugin("TestPlugin1");
    auto plugin2 = pluginLoader->loadPlugin("TestPlugin2");
    ASSERT_TRUE(plugin1 != nullptr);
    ASSERT_TRUE(plugin2 != nullptr);

    // Register plugins
    auto id1 = pluginManager->registerPlugin(std::move(plugin1));
    auto id2 = pluginManager->registerPlugin(std::move(plugin2));
    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);

    // Test plugin enable/disable
    EXPECT_TRUE(pluginManager->enablePlugin(id1));
    EXPECT_TRUE(pluginManager->disablePlugin(id2));

    // Verify plugin states
    EXPECT_TRUE(pluginManager->isPluginEnabled(id1));
    EXPECT_FALSE(pluginManager->isPluginEnabled(id2));

    // Test plugin unregistration
    EXPECT_TRUE(pluginManager->unregisterPlugin(id1));
    EXPECT_TRUE(pluginManager->unregisterPlugin(id2));

    // Verify plugins are unregistered
    EXPECT_FALSE(pluginManager->isPluginEnabled(id1));
    EXPECT_FALSE(pluginManager->isPluginEnabled(id2));
}

TEST_F(PluginSystemIntegrationTest, ConcurrentPluginOperations) {
    const int numThreads = 4;
    std::vector<std::future<bool>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            try {
                auto plugin = pluginLoader->loadPlugin("TestPlugin" + std::to_string(i));
                if (!plugin) return false;

                auto id = pluginManager->registerPlugin(std::move(plugin));
                if (id <= 0) return false;

                // Perform some operations
                bool success = pluginManager->enablePlugin(id);
                std::this_thread::sleep_for(10ms);
                success &= pluginManager->disablePlugin(id);
                success &= pluginManager->unregisterPlugin(id);

                return success;
            } catch (const std::exception& e) {
                std::cerr << "Plugin operation failed: " << e.what() << std::endl;
                return false;
            }
        }));
    }

    // All operations should succeed
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

TEST_F(PluginSystemIntegrationTest, PluginInteractionAndConflictResolution) {
    // Load multiple interacting plugins
    auto plugin1 = pluginLoader->loadPlugin("InteractingPlugin1");
    auto plugin2 = pluginLoader->loadPlugin("InteractingPlugin2");
    ASSERT_TRUE(plugin1 != nullptr);
    ASSERT_TRUE(plugin2 != nullptr);

    // Register plugins with dependencies
    auto id1 = pluginManager->registerPlugin(std::move(plugin1));
    auto id2 = pluginManager->registerPlugin(std::move(plugin2), {id1}); // plugin2 depends on plugin1
    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);

    // Enable plugins
    EXPECT_TRUE(pluginManager->enablePlugin(id1));
    EXPECT_TRUE(pluginManager->enablePlugin(id2));

    // Create test score
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    auto score = Score::create(timeSignature);
    auto voice = Voice::create(timeSignature);
    voice->addNote(60, 1.0);
    voice->addNote(64, 1.0);
    score->addVoice(std::move(voice));

    // Validate with interacting plugins
    bool isValid = validationPipeline->validate(*score);
    EXPECT_TRUE(isValid);

    // Test conflict resolution by disabling dependency
    EXPECT_FALSE(pluginManager->disablePlugin(id1)) 
        << "Should not be able to disable plugin1 while plugin2 depends on it";

    // Properly remove plugins in correct order
    EXPECT_TRUE(pluginManager->disablePlugin(id2));
    EXPECT_TRUE(pluginManager->disablePlugin(id1));
    EXPECT_TRUE(pluginManager->unregisterPlugin(id2));
    EXPECT_TRUE(pluginManager->unregisterPlugin(id1));
}

TEST_F(PluginSystemIntegrationTest, ErrorHandling) {
    // Test loading non-existent plugin
    EXPECT_THROW(pluginLoader->loadPlugin("NonExistentPlugin"), MusicTrainer::PluginError);

    // Test loading invalid plugin
    EXPECT_THROW(pluginLoader->loadPlugin("InvalidPlugin"), MusicTrainer::PluginError);

    // Test registering null plugin
    EXPECT_THROW(pluginManager->registerPlugin(nullptr), MusicTrainer::PluginError);

    // Test operations on invalid plugin ID
    EXPECT_FALSE(pluginManager->enablePlugin(-1));
    EXPECT_FALSE(pluginManager->disablePlugin(-1));
    EXPECT_FALSE(pluginManager->unregisterPlugin(-1));

    // Test circular dependencies
    auto plugin1 = pluginLoader->loadPlugin("CircularPlugin1");
    auto plugin2 = pluginLoader->loadPlugin("CircularPlugin2");
    ASSERT_TRUE(plugin1 != nullptr);
    ASSERT_TRUE(plugin2 != nullptr);

    auto id1 = pluginManager->registerPlugin(std::move(plugin1));
    EXPECT_THROW(pluginManager->registerPlugin(std::move(plugin2), {id1, id1}), 
                 MusicTrainer::PluginError);
}

} // namespace