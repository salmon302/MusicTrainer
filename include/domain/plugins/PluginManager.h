#ifndef MUSICTRAINERV3_PLUGINMANAGER_H
#define MUSICTRAINERV3_PLUGINMANAGER_H

#include "PluginInterface.h"
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace music::utils {
	class TrackedUniqueLock;
	enum class LockLevel;
}

namespace music::plugins {

class PluginManager {
public:
	static std::unique_ptr<PluginManager> create();
	~PluginManager() = default;

	// Plugin loading and unloading
	bool loadPlugin(const std::filesystem::path& pluginPath);
	void unloadPlugin(const std::string& pluginName);
	void unloadAllPlugins();

	// Plugin registration and management
	bool registerPlugin(const std::string& name, std::unique_ptr<PluginInterface> plugin);
	bool reloadPlugin(const std::string& name, std::unique_ptr<PluginInterface> plugin);
	std::vector<std::string> getLoadOrder() const;

	// Plugin access
	PluginInterface* getPlugin(const std::string& name);
	std::vector<std::string> getLoadedPlugins() const;

	// Extension point management
	bool registerExtensionPoint(const std::string& point, void* implementation);
	std::vector<std::string> getRegisteredExtensionPoints() const;

#ifdef TESTING
	std::shared_mutex& getMutexForTesting() const { return mutex; }
#endif

private:
	PluginManager() = default;
	
	struct PluginInfo {
		std::unique_ptr<PluginInterface> plugin;
		void* handle{nullptr};  // For dynamic library handle
		size_t loadOrder{0};    // For tracking load order
		bool initialized{false}; // Track initialization state

		PluginInfo() = default;
		PluginInfo(PluginInfo&&) = default;
		PluginInfo& operator=(PluginInfo&&) = default;
	};

	mutable std::shared_mutex mutex;  // Reader-writer lock for thread safety
	std::unordered_map<std::string, PluginInfo> loadedPlugins;
	std::unordered_map<std::string, void*> extensionPoints;
	std::atomic<size_t> nextLoadOrder{0};
};

} // namespace music::plugins

#endif // MUSICTRAINERV3_PLUGINMANAGER_H
