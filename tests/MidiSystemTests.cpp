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
	
	music::ports::MidiEvent event(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	for (int i = 0; i < 10; i++) {
		adapter->sendEvent(event);
		std::this_thread::sleep_for(milliseconds(10));
	}
	
	auto metrics = adapter->getMetrics();
	EXPECT_EQ(metrics.totalEvents, 10);
	EXPECT_EQ(metrics.errorCount, 0);
	EXPECT_EQ(metrics.recoveredErrors, 0);
	EXPECT_GT(metrics.maxLatencyUs, 0.0);
	
	adapter->close();
}

TEST(MidiSystemTest, ErrorRecovery) {
	auto adapter = MockMidiAdapter::create();
	adapter->setSimulateErrors(true);
	
	ASSERT_TRUE(adapter->open());
	
	music::ports::MidiEvent event(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	adapter->sendEvent(event);
	auto metrics = adapter->getMetrics();
	EXPECT_GT(metrics.recoveredErrors, 0);
	
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
	
	int errorCount = 0;
	for (int i = 0; i < 10; i++) {
		try {
			adapter->sendEvent(event);
		} catch (const std::runtime_error& e) {
			errorCount++;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	auto metrics = adapter->getMetrics();
	EXPECT_GT(errorCount, 0);
	EXPECT_LT(metrics.recoveredErrors, errorCount);
	
	adapter->close();
}

TEST(MidiSystemTest, PriorityBasedProcessing) {
	auto adapter = MockMidiAdapter::create();
	ASSERT_TRUE(adapter->open());
	
	std::vector<music::ports::MidiEvent::Type> processedEvents;
	std::atomic<bool> processingComplete{false};
	std::atomic<int> processedCount{0};
	
	adapter->setEventCallback([&](const music::ports::MidiEvent& evt) {
		std::cout << "Processing event type: " << static_cast<int>(evt.type) << std::endl;
		processedEvents.push_back(evt.type);
		processedCount.fetch_add(1, std::memory_order_release);
		if (processedCount.load(std::memory_order_acquire) == 2) {
			processingComplete.store(true, std::memory_order_release);
		}
	});
	
	music::ports::MidiEvent control(music::ports::MidiEvent::Type::CONTROL_CHANGE, 0, 7, 100);
	music::ports::MidiEvent noteOn(music::ports::MidiEvent::Type::NOTE_ON, 0, 60, 100);
	
	std::cout << "Sending both events..." << std::endl;
	adapter->sendEvent(control);
	adapter->sendEvent(noteOn);
	
	// Wait for processing to complete
	auto start = std::chrono::steady_clock::now();
	while (!processingComplete.load(std::memory_order_acquire)) {
		if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
			FAIL() << "Timeout waiting for events to be processed";
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	std::cout << "Processed events order:" << std::endl;
	for (size_t i = 0; i < processedEvents.size(); ++i) {
		std::cout << "Event " << i << " type: " << static_cast<int>(processedEvents[i]) << std::endl;
	}
	
	ASSERT_EQ(processedEvents.size(), 2);
	EXPECT_EQ(processedEvents[0], music::ports::MidiEvent::Type::NOTE_ON);
	EXPECT_EQ(processedEvents[1], music::ports::MidiEvent::Type::CONTROL_CHANGE);
	
	adapter->close();
}

