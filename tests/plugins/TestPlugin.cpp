#include "TestPlugin.h"
#include <iostream>


namespace music::test {

std::unique_ptr<TestPlugin> TestPlugin::create() {
	return std::unique_ptr<TestPlugin>(new TestPlugin());
}

bool TestPlugin::initialize() {
	bool expected = false;
	// Only allow initialization if not previously initialized
	if (initialized.compare_exchange_strong(expected, true)) {
		std::cout << "[TestPlugin] Successfully initialized" << std::endl;
		return true;
	}
	std::cout << "[TestPlugin] Already initialized, initialization failed" << std::endl;
	return false;
}


void TestPlugin::shutdown() {
	initialized.store(false);
	reloadPrepared = false;
	state.clear();
}


std::vector<std::string> TestPlugin::getExtensionPoints() const {
	return {"test.extension.point1", "test.extension.point2"};
}

bool TestPlugin::registerExtensionPoint(const std::string& point, void* implementation) {
	if (!initialized) return false;
	if (point != "test.extension.point1" && point != "test.extension.point2") return false;
	return true;
}

void TestPlugin::setState(const std::string& key, const std::string& value) {
	state[key] = value;
}

std::optional<std::string> TestPlugin::getState(const std::string& key) const {
	auto it = state.find(key);
	if (it != state.end()) {
		return it->second;
	}
	return std::nullopt;
}

bool TestPlugin::prepareForReload() {
	if (!initialized) return false;
	reloadPrepared = true;
	return true;
}

bool TestPlugin::finalizeReload() {
	if (!reloadPrepared) return false;
	reloadPrepared = false;
	return true;
}

} // namespace music::test