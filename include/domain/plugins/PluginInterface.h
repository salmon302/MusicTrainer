#ifndef MUSICTRAINERV3_PLUGININTERFACE_H
#define MUSICTRAINERV3_PLUGININTERFACE_H

#include <string>
#include <memory>
#include <vector>
#include <optional>

namespace music::plugins {

class PluginInterface {
public:
	virtual ~PluginInterface() = default;
	
	// Plugin metadata
	virtual std::string getName() const = 0;
	virtual std::string getVersion() const = 0;
	virtual std::string getDescription() const = 0;
	
	// Lifecycle management
	virtual bool initialize() = 0;
	virtual void shutdown() = 0;
	
	// Resource management
	virtual bool requiresIsolation() const = 0;
	virtual size_t getMemoryLimit() const = 0;
	
	// Extension point registration
	virtual std::vector<std::string> getExtensionPoints() const = 0;
	virtual bool registerExtensionPoint(const std::string& point, void* implementation) = 0;
	
	// State management
	virtual void setState(const std::string& key, const std::string& value) = 0;
	virtual std::optional<std::string> getState(const std::string& key) const = 0;
	
	// Hot reload support
	virtual bool prepareForReload() = 0;
	virtual bool finalizeReload() = 0;
};

// Plugin factory function type
using CreatePluginFunc = std::unique_ptr<PluginInterface> (*)();

} // namespace music::plugins

#endif // MUSICTRAINERV3_PLUGININTERFACE_H