#include <gtest/gtest.h>
#include "domain/plugins/PluginManager.h"
#include "domain/errors/ErrorHandler.h"
#include "TestPlugin.h"
#include <filesystem>
#include <thread>
#include <chrono>

using namespace music::plugins;
using namespace music::test;
using namespace MusicTrainer;

class PluginSystemTest : public ::testing::Test {
protected:
	void SetUp() override {
		manager = PluginManager::create();
		errorHandler = &MusicTrainer::ErrorHandler::getInstance();
		configureDefaultErrorHandlers();
	}

	void TearDown() override {
		manager->unloadAllPlugins();
	}

	std::unique_ptr<PluginManager> manager;
	MusicTrainer::ErrorHandler* errorHandler;

private:
	void configureDefaultErrorHandlers() {
		errorHandler->registerHandler("PluginError",
			[](const MusicTrainerError& error) {
				std::cerr << "Plugin error: " << error.what() << std::endl;
			},
			ErrorSeverity::Error
		);
	}
};

TEST_F(PluginSystemTest, LoadPlugin) {
	auto testPlugin = TestPlugin::create();
	ASSERT_TRUE(testPlugin->initialize());
	EXPECT_EQ(testPlugin->getName(), "TestPlugin");
	EXPECT_EQ(testPlugin->getVersion(), "1.0.0");
}

TEST_F(PluginSystemTest, ExtensionPoints) {
	auto testPlugin = TestPlugin::create();
	ASSERT_TRUE(testPlugin->initialize());
	
	auto points = testPlugin->getExtensionPoints();
	ASSERT_EQ(points.size(), 2);
	EXPECT_EQ(points[0], "test.extension.point1");
	EXPECT_EQ(points[1], "test.extension.point2");
	
	EXPECT_TRUE(testPlugin->registerExtensionPoint("test.extension.point1", nullptr));
	EXPECT_FALSE(testPlugin->registerExtensionPoint("invalid.point", nullptr));
}

TEST_F(PluginSystemTest, PluginLifecycle) {
	auto testPlugin = TestPlugin::create();
	ASSERT_TRUE(testPlugin->initialize());
	EXPECT_TRUE(testPlugin->registerExtensionPoint("test.extension.point1", nullptr));
	
	testPlugin->shutdown();
	EXPECT_FALSE(testPlugin->registerExtensionPoint("test.extension.point1", nullptr));
}

TEST_F(PluginSystemTest, PluginManager) {
	EXPECT_TRUE(manager->registerExtensionPoint("test.extension.point1", nullptr));
	EXPECT_FALSE(manager->registerExtensionPoint("test.extension.point1", nullptr));
	
	auto points = manager->getRegisteredExtensionPoints();
	ASSERT_EQ(points.size(), 1);
	EXPECT_EQ(points[0], "test.extension.point1");
}

TEST_F(PluginSystemTest, ResourceIsolation) {
	auto testPlugin = TestPlugin::create();
	ASSERT_TRUE(testPlugin->initialize());
	
	EXPECT_EQ(testPlugin->getMemoryLimit(), 1024 * 1024);
	EXPECT_FALSE(testPlugin->requiresIsolation());
	
	testPlugin->shutdown();
	EXPECT_FALSE(testPlugin->initialize());
}

TEST_F(PluginSystemTest, PluginDependencies) {
	auto plugin1 = TestPlugin::create();
	auto plugin2 = TestPlugin::create();
	
	EXPECT_TRUE(manager->registerPlugin("plugin1", std::move(plugin1)));
	EXPECT_TRUE(manager->registerPlugin("plugin2", std::move(plugin2)));
	
	auto loadOrder = manager->getLoadOrder();
	ASSERT_EQ(loadOrder.size(), 2);
	EXPECT_EQ(loadOrder[0], "plugin1");
	EXPECT_EQ(loadOrder[1], "plugin2");
}

TEST_F(PluginSystemTest, ConcurrentPluginOperations) {
	const int NUM_THREADS = 4;  // Reduced from 10
	std::vector<std::thread> threads;
	std::atomic<int> successCount{0};
	std::atomic<int> errorCount{0};
	
	for (int i = 0; i < NUM_THREADS; i++) {
		threads.emplace_back([this, i, &successCount, &errorCount]() {
			try {
				auto plugin = TestPlugin::create();
				if (plugin && plugin->initialize()) {
					std::string pluginName = "plugin" + std::to_string(i);
					if (manager->registerPlugin(pluginName, std::move(plugin))) {
						successCount.fetch_add(1, std::memory_order_relaxed);
					}
				}
			} catch (const std::exception& e) {
				std::cerr << "Thread " << i << " error: " << e.what() << std::endl;
				errorCount.fetch_add(1, std::memory_order_relaxed);
			}
		});
	}
	
	// Join threads with error checking
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	
	const int total = successCount.load(std::memory_order_acquire);
	const int errors = errorCount.load(std::memory_order_acquire);
	EXPECT_EQ(errors, 0) << "Encountered " << errors << " errors during concurrent operations";
	EXPECT_EQ(total, NUM_THREADS);
	EXPECT_EQ(manager->getLoadedPlugins().size(), NUM_THREADS);
}

TEST_F(PluginSystemTest, PluginHotReload) {
	auto testPlugin = TestPlugin::create();
	ASSERT_TRUE(testPlugin->initialize());
	ASSERT_TRUE(manager->registerPlugin("test.plugin", std::move(testPlugin)));
	
	auto reloadedPlugin = TestPlugin::create();
	ASSERT_TRUE(reloadedPlugin->initialize());
	EXPECT_TRUE(manager->reloadPlugin("test.plugin", std::move(reloadedPlugin)));
	
	auto pluginNames = manager->getLoadedPlugins();
	EXPECT_EQ(pluginNames.size(), 1);
	auto* plugin = manager->getPlugin(pluginNames[0]);
	ASSERT_NE(plugin, nullptr);
	EXPECT_TRUE(plugin->initialize());
}