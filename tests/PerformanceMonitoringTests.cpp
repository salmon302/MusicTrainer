#include <gtest/gtest.h>
#include "domain/monitoring/PerformanceMonitor.h"
#include <thread>
#include <chrono>

using namespace domain::monitoring;

class PerformanceMonitoringTest : public ::testing::Test {
protected:
	void SetUp() override {
		auto& monitor = PerformanceMonitor::getInstance();
		// Reset initial metrics
		monitor.updateResourceMetrics(0, 0.0, 0.0, 0.0);
		monitor.updateHealthMetrics(0.0, 0.0, 0.0);
	}
};

TEST_F(PerformanceMonitoringTest, RecordsLatencyMetrics) {
	auto& monitor = PerformanceMonitor::getInstance();
	
	// Record test latencies
	monitor.recordLatency("midi_input", 5.0);
	monitor.recordLatency("event_processing", 3.0);
	monitor.recordLatency("rule_validation", 25.0);

	auto metrics = monitor.getMetrics();
	EXPECT_DOUBLE_EQ(metrics.midiInputLatency, 5.0);
	EXPECT_DOUBLE_EQ(metrics.eventProcessingTime, 3.0);
	EXPECT_DOUBLE_EQ(metrics.ruleValidationTime, 25.0);
}

TEST_F(PerformanceMonitoringTest, UpdatesResourceMetrics) {
	auto& monitor = PerformanceMonitor::getInstance();
	
	monitor.updateResourceMetrics(256000, 45.5, 0.85, 0.95);
	
	auto metrics = monitor.getMetrics();
	EXPECT_EQ(metrics.peakMemoryUsage, 256000);
	EXPECT_DOUBLE_EQ(metrics.cpuUtilization, 45.5);
	EXPECT_DOUBLE_EQ(metrics.threadEfficiency, 0.85);
	EXPECT_DOUBLE_EQ(metrics.cacheHitRate, 0.95);
}

TEST_F(PerformanceMonitoringTest, UpdatesHealthMetrics) {
	auto& monitor = PerformanceMonitor::getInstance();
	
	monitor.updateHealthMetrics(0.001, 50.0, 3600.0);
	
	auto metrics = monitor.getMetrics();
	EXPECT_DOUBLE_EQ(metrics.errorRate, 0.001);
	EXPECT_DOUBLE_EQ(metrics.recoveryTime, 50.0);
	EXPECT_DOUBLE_EQ(metrics.uptime, 3600.0);
}

TEST_F(PerformanceMonitoringTest, ChecksPerformanceThresholds) {
	auto& monitor = PerformanceMonitor::getInstance();
	
	// Set metrics within acceptable thresholds
	monitor.recordLatency("midi_input", 8.0);
	monitor.recordLatency("event_processing", 4.0);
	monitor.recordLatency("rule_validation", 45.0);
	monitor.updateHealthMetrics(0.0005, 80.0, 3600.0);
	monitor.updateResourceMetrics(256000, 45.5, 0.85, 0.95);
	
	auto metrics = monitor.getMetrics();
	std::cout << "Checking metrics:\n"
			  << "Latencies: " << metrics.midiInputLatency << ", " 
			  << metrics.eventProcessingTime << ", " 
			  << metrics.ruleValidationTime << "\n"
			  << "Health: " << metrics.errorRate << ", "
			  << metrics.recoveryTime << ", "
			  << metrics.uptime << "\n"
			  << "Resources: " << metrics.peakMemoryUsage << ", "
			  << metrics.cpuUtilization << ", "
			  << metrics.threadEfficiency << ", "
			  << metrics.cacheHitRate << "\n";
	
	EXPECT_TRUE(monitor.checkPerformanceThresholds());
	
	// Set metrics outside acceptable thresholds
	monitor.recordLatency("midi_input", 15.0);
	EXPECT_FALSE(monitor.checkPerformanceThresholds());
}