#include "domain/monitoring/PerformanceMonitor.h"

namespace domain::monitoring {

PerformanceMonitor& PerformanceMonitor::getInstance() {
	static PerformanceMonitor instance;
	return instance;
}

void PerformanceMonitor::startMeasurement(const std::string& operationId) {
	activeOperations[operationId] = std::chrono::steady_clock::now();
}

void PerformanceMonitor::endMeasurement(const std::string& operationId) {
	auto endTime = std::chrono::steady_clock::now();
	auto startTime = activeOperations[operationId];
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	measurements[operationId] = duration.count() / 1000.0; // Convert to milliseconds
	activeOperations.erase(operationId);
}

double PerformanceMonitor::getLatency(const std::string& operationId) const {
	auto it = measurements.find(operationId);
	return it != measurements.end() ? it->second : 0.0;
}

void PerformanceMonitor::reset() {
	activeOperations.clear();
	measurements.clear();
	metrics = PerformanceMetrics(); // Reset metrics to default values
}

void PerformanceMonitor::recordLatency(const std::string& operation, double latency) {
	if (operation == "midi_input") {
		metrics.midiInputLatency = latency;
	} else if (operation == "event_processing") {
		metrics.eventProcessingTime = latency;
	} else if (operation == "rule_validation") {
		metrics.ruleValidationTime = latency;
	}
	measurements[operation] = latency;
}

void PerformanceMonitor::updateResourceMetrics(double memory, double cpu, double threadEff, double cacheHit) {
	metrics.peakMemoryUsage = memory;
	metrics.cpuUtilization = cpu;
	metrics.threadEfficiency = threadEff;
	metrics.cacheHitRate = cacheHit;
}

void PerformanceMonitor::updateHealthMetrics(double error, double recovery, double up) {
	metrics.errorRate = error;
	metrics.recoveryTime = recovery;
	metrics.uptime = up;
}

bool PerformanceMonitor::checkPerformanceThresholds() const {
	// Check latency thresholds
	if (metrics.midiInputLatency > 10.0 ||
		metrics.eventProcessingTime > 5.0 ||
		metrics.ruleValidationTime > 50.0) return false;

	// Check resource thresholds
	if (metrics.peakMemoryUsage > 512000 ||  // 512MB in KB
		metrics.cpuUtilization > 70.0 ||
		metrics.threadEfficiency < 0.80 ||    // 80% as decimal
		metrics.cacheHitRate < 0.90) return false;  // 90% as decimal

	// Check health thresholds
	if (metrics.errorRate > 0.001 ||      // 0.1%
		metrics.recoveryTime > 100.0 ||    // 100ms
		metrics.uptime < 3600.0) return false;  // 1 hour in seconds

	return true;
}

const PerformanceMetrics& PerformanceMonitor::getMetrics() const {
    return metrics;
}

} // namespace domain::monitoring