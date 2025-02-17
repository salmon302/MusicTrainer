#include <gtest/gtest.h>
#include "adapters/RtMidiAdapter.h"
#include "adapters/MockMidiAdapter.h"
#include "domain/ports/MidiPort.h"
#include <thread>
#include <chrono>

using namespace music::adapters;
using namespace std::chrono;

TEST(MidiSystemTest, MetricsTracking) {
	auto adapter = MockMidiAdapter::create();
	ASSERT_TRUE(adapter->open());
	
	// Send test events
	music::ports::MidiEvent event(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	for (int i = 0; i < 10; i++) {
		adapter->sendEvent(event);
		std::this_thread::sleep_for(milliseconds(10));
	}
	
	// Check metrics
	auto metrics = adapter->getMetrics();
	EXPECT_EQ(metrics.totalEvents, 10);
	EXPECT_EQ(metrics.errorCount, 0);
	EXPECT_EQ(metrics.recoveredErrors, 0);
	EXPECT_GT(metrics.maxLatencyUs, 0.0);
	
	adapter->close();
}

TEST(MidiSystemTest, ErrorRecovery) {
	auto adapter = MockMidiAdapter::create();
	adapter->setSimulateErrors(true);  // Enable error simulation
	
	ASSERT_TRUE(adapter->open());
	
	music::ports::MidiEvent event(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	// Test soft reset recovery
	adapter->sendEvent(event);  // This should be a recoverable error
	auto metrics = adapter->getMetrics();
	EXPECT_GT(metrics.recoveredErrors, 0);
	
	// Test hard reset after multiple errors
	bool hardResetTriggered = false;
	for (int i = 0; i < 5; i++) {
		try {
			adapter->sendEvent(event);
		} catch (const std::runtime_error& e) {
			hardResetTriggered = true;
			break;
		}
	}
	EXPECT_TRUE(hardResetTriggered);
	
	metrics = adapter->getMetrics();
	EXPECT_GT(metrics.errorCount, 0);
	EXPECT_GT(metrics.recoveredErrors, 0);
	
	adapter->close();
}

TEST(MidiSystemTest, ErrorWindowThrottling) {

	auto adapter = MockMidiAdapter::create();
	adapter->setSimulateErrors(true);
	
	ASSERT_TRUE(adapter->open());
	
	music::ports::MidiEvent event(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	// Trigger multiple errors within error window
	int errorCount = 0;
	for (int i = 0; i < 10; i++) {
		try {
			adapter->sendEvent(event);
		} catch (const std::runtime_error& e) {
			errorCount++;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	// Verify error throttling
	auto metrics = adapter->getMetrics();
	EXPECT_GT(errorCount, 0);
	EXPECT_LT(metrics.recoveredErrors, errorCount); // Some errors should not be recovered due to throttling
	
	adapter->close();
}

TEST(MidiSystemTest, PriorityBasedProcessing) {
	auto adapter = MockMidiAdapter::create();
	ASSERT_TRUE(adapter->open());
	
	std::vector<music::ports::MidiEvent::Type> processedEvents;
	std::mutex eventMutex;
	std::condition_variable eventCV;
	std::atomic<int> processedCount{0};
	
	adapter->setEventCallback([&](const music::ports::MidiEvent& evt) {
		{
			std::lock_guard<std::mutex> lock(eventMutex);
			std::cout << "Processing event type: " << static_cast<int>(evt.type) << std::endl;
			processedEvents.push_back(evt.type);
			processedCount.fetch_add(1, std::memory_order_release);
		}
		eventCV.notify_one();
	});
	
	// Create events
	music::ports::MidiEvent control(music::ports::MidiEvent::Type::CONTROL_CHANGE, 0, 7, 100);
	music::ports::MidiEvent noteOn(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	// Send both events
	std::cout << "Sending both events..." << std::endl;
	adapter->sendEvent(control);
	adapter->sendEvent(noteOn);
	
	// Wait for both events to be processed
	{
		std::unique_lock<std::mutex> lock(eventMutex);
		bool success = eventCV.wait_for(lock, std::chrono::seconds(1), 
			[&processedCount]() { return processedCount.load(std::memory_order_acquire) == 2; });
		ASSERT_TRUE(success) << "Timeout waiting for events to be processed";
		
		std::cout << "Processed events order:" << std::endl;
		for (size_t i = 0; i < processedEvents.size(); ++i) {
			std::cout << "Event " << i << " type: " << static_cast<int>(processedEvents[i]) << std::endl;
		}
		
		ASSERT_EQ(processedEvents.size(), 2);
		EXPECT_EQ(processedEvents[0], music::ports::MidiEvent::Type::NOTE_ON);
		EXPECT_EQ(processedEvents[1], music::ports::MidiEvent::Type::CONTROL_CHANGE);
	}
	
	adapter->close();
}
