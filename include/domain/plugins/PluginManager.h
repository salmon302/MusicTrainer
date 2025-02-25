#ifndef MUSICTRAINERV3_PLUGINMANAGER_H
#define MUSICTRAINERV3_PLUGINMANAGER_H

#include "PluginInterface.h"
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <atomic>
#include <memory>

namespace music::plugins {

class PluginManager {
public:
	static std::unique_ptr<PluginManager> create();
	~PluginManager() = default;

	bool loadPlugin(const std::filesystem::path& pluginPath);
	void unloadPlugin(const std::string& pluginName);
	void unloadAllPlugins();
	bool registerPlugin(const std::string& name, std::unique_ptr<PluginInterface> plugin);
	bool reloadPlugin(const std::string& name, std::unique_ptr<PluginInterface> plugin);
	std::vector<std::string> getLoadOrder() const;
	PluginInterface* getPlugin(const std::string& name);
	std::vector<std::string> getLoadedPlugins() const;
	bool registerExtensionPoint(const std::string& point, void* implementation);
	std::vector<std::string> getRegisteredExtensionPoints() const;

private:
	PluginManager() = default;
	
	struct PluginInfo {
		std::unique_ptr<PluginInterface> plugin;
		void* handle{nullptr};
		std::atomic<size_t> loadOrder{0};
		std::atomic<bool> initialized{false};

		PluginInfo() = default;
		PluginInfo(PluginInfo&&) = default;
		PluginInfo& operator=(PluginInfo&&) = default;
	};

	std::unordered_map<std::string, std::unique_ptr<PluginInfo>> loadedPlugins;
	std::unordered_map<std::string, void*> extensionPoints;
	std::atomic<size_t> nextLoadOrder{0};
};

} // namespace music::plugins

#endif // MUSICTRAINERV3_PLUGINMANAGER_H

