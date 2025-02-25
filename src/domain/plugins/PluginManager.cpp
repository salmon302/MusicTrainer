#include "domain/plugins/PluginManager.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <stdexcept>
#include <iostream>
#include <future>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace music;
using MusicTrainer::DomainError;

namespace music::plugins {

std::unique_ptr<PluginManager> PluginManager::create() {
	return std::unique_ptr<PluginManager>(new PluginManager());
}

bool PluginManager::loadPlugin(const std::filesystem::path& pluginPath) {
	try {
		void* handle = nullptr;
		#ifdef _WIN32
		handle = LoadLibraryA(pluginPath.string().c_str());
		#else
		handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
		#endif
		
		if (!handle) {
			throw DomainError("Failed to load plugin: " + pluginPath.string(), "PluginError");
		}
		
		#ifdef _WIN32
		auto createPlugin = reinterpret_cast<CreatePluginFunc>(
			GetProcAddress(static_cast<HMODULE>(handle), "createPlugin"));
		#else
		auto createPlugin = reinterpret_cast<CreatePluginFunc>(dlsym(handle, "createPlugin"));
		#endif
		
		if (!createPlugin) {
			#ifdef _WIN32
			FreeLibrary(static_cast<HMODULE>(handle));
			#else
			dlclose(handle);
			#endif
			throw DomainError("Invalid plugin format: createPlugin function not found", "PluginError");
		}
		
		auto plugin = createPlugin();
		if (!plugin || !plugin->initialize()) {
			#ifdef _WIN32
			FreeLibrary(static_cast<HMODULE>(handle));
			#else
			dlclose(handle);
			#endif
			throw DomainError("Plugin initialization failed", "PluginError");
		}
		
		auto pluginInfo = std::make_unique<PluginInfo>();
		pluginInfo->plugin = std::move(plugin);
		pluginInfo->handle = handle;
		pluginInfo->loadOrder = nextLoadOrder++;
		pluginInfo->initialized = true;
		
		loadedPlugins[pluginInfo->plugin->getName()] = std::move(pluginInfo);
		return true;
	} catch (const std::exception& e) {
		DomainError domainError("Plugin loading failed: " + std::string(e.what()), "PluginError");
		HANDLE_ERROR(domainError);
		return false;
	}
}

void PluginManager::unloadPlugin(const std::string& pluginName) {
	try {
		auto it = loadedPlugins.find(pluginName);
		if (it == loadedPlugins.end()) {
			throw DomainError("Plugin not found: " + pluginName, "PluginError");
		}
		
		it->second->plugin->shutdown();
		#ifdef _WIN32
		if (it->second->handle) {
			FreeLibrary(static_cast<HMODULE>(it->second->handle));
		}
		#else
		if (it->second->handle) {
			dlclose(it->second->handle);
		}
		#endif
		
		loadedPlugins.erase(pluginName);
	} catch (const std::exception& e) {
		DomainError domainError("Plugin unloading failed: " + std::string(e.what()), "PluginError");
		HANDLE_ERROR(domainError);
	}
}

void PluginManager::unloadAllPlugins() {
	for (const auto& [name, info] : loadedPlugins) {
		info->plugin->shutdown();
		#ifdef _WIN32
		if (info->handle) {
			FreeLibrary(static_cast<HMODULE>(info->handle));
		}
		#else
		if (info->handle) {
			dlclose(info->handle);
		}
		#endif
	}
	loadedPlugins.clear();
}

PluginInterface* PluginManager::getPlugin(const std::string& name) {
	auto it = loadedPlugins.find(name);
	return it != loadedPlugins.end() ? it->second->plugin.get() : nullptr;
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
	std::vector<std::string> result;
	result.reserve(loadedPlugins.size());
	for (const auto& [name, _] : loadedPlugins) {
		result.push_back(name);
	}
	return result;
}

bool PluginManager::registerExtensionPoint(const std::string& point, void* implementation) {
	if (extensionPoints.find(point) != extensionPoints.end()) {
		return false;
	}
	extensionPoints[point] = implementation;
	return true;
}

std::vector<std::string> PluginManager::getRegisteredExtensionPoints() const {
	std::vector<std::string> result;
	result.reserve(extensionPoints.size());
	for (const auto& [point, _] : extensionPoints) {
		result.push_back(point);
	}
	return result;
}

bool PluginManager::registerPlugin(const std::string& name, std::unique_ptr<PluginInterface> plugin) {
	try {
		if (!plugin) {
			throw DomainError("Invalid plugin pointer", "PluginError");
		}
		
		if (!plugin->initialize()) {
			return false;
		}
		
		auto pluginInfo = std::make_unique<PluginInfo>();
		pluginInfo->plugin = std::move(plugin);
		pluginInfo->loadOrder = nextLoadOrder++;
		pluginInfo->initialized = true;
		
		if (loadedPlugins.find(name) != loadedPlugins.end()) {
			return false;
		}
		loadedPlugins[name] = std::move(pluginInfo);
		return true;
	} catch (const std::exception& e) {
		DomainError domainError("Plugin registration failed: " + std::string(e.what()), "PluginError");
		HANDLE_ERROR(domainError);
		return false;
	}
}

bool PluginManager::reloadPlugin(const std::string& name, std::unique_ptr<PluginInterface> plugin) {
	try {
		auto it = loadedPlugins.find(name);
		if (it == loadedPlugins.end()) {
			throw DomainError("Plugin not found: " + name, "PluginError");
		}
		
		if (!plugin->prepareForReload()) {
			throw DomainError("Plugin reload preparation failed", "PluginError");
		}
		
		size_t oldLoadOrder = it->second->loadOrder;
		it->second->plugin->shutdown();
		
		if (!plugin->initialize() || !plugin->finalizeReload()) {
			throw DomainError("Plugin reload failed", "PluginError");
		}
		
		auto pluginInfo = std::make_unique<PluginInfo>();
		pluginInfo->plugin = std::move(plugin);
		pluginInfo->loadOrder = oldLoadOrder;
		pluginInfo->initialized = true;
		
		loadedPlugins[name] = std::move(pluginInfo);
		return true;
	} catch (const std::exception& e) {
		DomainError domainError("Plugin reload failed: " + std::string(e.what()), "PluginError");
		HANDLE_ERROR(domainError);
		return false;
	}
}

std::vector<std::string> PluginManager::getLoadOrder() const {
	std::vector<std::pair<std::string, size_t>> ordered;
	ordered.reserve(loadedPlugins.size());
	
	for (const auto& [name, info] : loadedPlugins) {
		ordered.emplace_back(name, info->loadOrder);
	}
	
	std::sort(ordered.begin(), ordered.end(),
			  [](const auto& a, const auto& b) { return a.second < b.second; });
	
	std::vector<std::string> result;
	result.reserve(ordered.size());
	for (const auto& [name, _] : ordered) {
		result.push_back(name);
	}
	return result;
}

} // namespace music::plugins
