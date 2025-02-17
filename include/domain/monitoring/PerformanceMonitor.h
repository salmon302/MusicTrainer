#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <memory>

namespace domain::monitoring {

struct PerformanceMetrics {
	// Latency metrics
	double midiInputLatency = 0.0;
	double eventProcessingTime = 0.0;
	double ruleValidationTime = 0.0;
	
	// Resource metrics
	double peakMemoryUsage = 0.0;
	double cpuUtilization = 0.0;
	double threadEfficiency = 0.0;
	double cacheHitRate = 0.0;
	
	// Health metrics
	double errorRate = 0.0;
	double recoveryTime = 0.0;
	double uptime = 0.0;
};

class PerformanceMonitor {
public:
	static PerformanceMonitor& getInstance();
	
	// Existing timing methods
	void startMeasurement(const std::string& operationId);
	void endMeasurement(const std::string& operationId);
	double getLatency(const std::string& operationId) const;
	void reset();

	// New monitoring methods
	void recordLatency(const std::string& operation, double latency);
	void updateResourceMetrics(double memory, double cpu, double threadEff, double cacheHit);
	void updateHealthMetrics(double error, double recovery, double up);
	bool checkPerformanceThresholds() const;
	const PerformanceMetrics& getMetrics() const;

private:
	PerformanceMonitor() = default;
	std::unordered_map<std::string, std::chrono::steady_clock::time_point> activeOperations;
	std::unordered_map<std::string, double> measurements;
	PerformanceMetrics metrics;
};

} // namespace domain::monitoring