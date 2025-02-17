#ifndef MUSICTRAINERV3_TESTPLUGIN_H
#define MUSICTRAINERV3_TESTPLUGIN_H

#include "domain/plugins/PluginInterface.h"
#include <unordered_map>
#include <optional>
#include <atomic>

namespace music::test {

class TestPlugin : public plugins::PluginInterface {
public:
	static std::unique_ptr<TestPlugin> create();
	~TestPlugin() override = default;

	std::string getName() const override { return "TestPlugin"; }
	std::string getVersion() const override { return "1.0.0"; }
	std::string getDescription() const override { return "Test plugin for unit testing"; }

	bool initialize() override;
	void shutdown() override;

	bool requiresIsolation() const override { return false; }
	size_t getMemoryLimit() const override { return 1024 * 1024; } // 1MB

	std::vector<std::string> getExtensionPoints() const override;
	bool registerExtensionPoint(const std::string& point, void* implementation) override;

	// State management
	void setState(const std::string& key, const std::string& value) override;
	std::optional<std::string> getState(const std::string& key) const override;
	
	// Hot reload support
	bool prepareForReload() override;
	bool finalizeReload() override;

private:
	TestPlugin() = default;
	std::atomic<bool> initialized{false};
	std::unordered_map<std::string, std::string> state;
	bool reloadPrepared{false};
};

} // namespace music::test

#endif // MUSICTRAINERV3_TESTPLUGIN_H