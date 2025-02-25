#ifndef MUSICTRAINERV3_LOCKFREEEVENTQUEUE_H
#define MUSICTRAINERV3_LOCKFREEEVENTQUEUE_H

#include <atomic>
#include <array>
#include <optional>
#include <algorithm>
#include "domain/ports/MidiPort.h"

namespace music::adapters {

template<typename EventType = ports::MidiEvent, size_t QueueSize = 1024>
class LockFreeEventQueue {
public:
	struct QueueMetrics {
		std::atomic<size_t> totalPushes{0};
		std::atomic<size_t> totalPops{0};
		std::atomic<size_t> pushFailures{0};
		std::atomic<size_t> maxQueueSize{0};
		std::atomic<uint64_t> totalLatencyUs{0};
		
		double getAverageLatencyUs() const {
			size_t total = totalPops.load();
			return total > 0 ? static_cast<double>(totalLatencyUs.load()) / total : 0.0;
		}
	};

	struct Entry {
		std::optional<EventType> event;
		std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
		int priority{0};
		uint64_t sequence{0};

		Entry() = default;
		Entry(EventType e, int p, uint64_t s) 
			: event(std::move(e)), priority(p), sequence(s) {}

		bool operator<(const Entry& other) const {
			if (priority != other.priority) return priority > other.priority;
			return sequence > other.sequence;  // LIFO within same priority
		}
	};

	LockFreeEventQueue() : nextSequence(0) {}

	bool push(EventType event, int priority = 0) {
		size_t current_tail = tail.load(std::memory_order_relaxed);
		size_t next_tail = (current_tail + 1) % QueueSize;
		
		if (next_tail == head.load(std::memory_order_acquire)) {
			metrics.pushFailures++;
			return false;
		}
		
		buffer[current_tail] = Entry(std::move(event), priority, nextSequence++);
		tail.store(next_tail, std::memory_order_release);
		
		size_t currentSize = (QueueSize + next_tail - head.load()) % QueueSize;
		metrics.maxQueueSize.store(std::max(metrics.maxQueueSize.load(), currentSize));
		metrics.totalPushes++;
		
		return true;

	}
	
	std::optional<EventType> pop() {
		size_t current_head = head.load(std::memory_order_relaxed);
		if (current_head == tail.load(std::memory_order_acquire)) {
			return std::nullopt;
		}

		// Find highest priority entry
		size_t best_index = current_head;
		size_t current_tail = tail.load(std::memory_order_acquire);
		
		for (size_t i = current_head; i != current_tail; i = (i + 1) % QueueSize) {
			if (buffer[i] < buffer[best_index]) {
				best_index = i;
			}
		}

		// If best entry isn't at head, swap it
		if (best_index != current_head) {
			std::swap(buffer[current_head], buffer[best_index]);
		}

		auto result = std::move(buffer[current_head].event);
		auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now() - buffer[current_head].timestamp
		).count();

		metrics.totalLatencyUs += latency;
		metrics.totalPops++;
		
		head.store((current_head + 1) % QueueSize, std::memory_order_release);
		return result;
	}

	bool empty() const {
		return head.load(std::memory_order_acquire) == 
			   tail.load(std::memory_order_acquire);
	}

	void clear() {
		// Reset head and tail atomically
		head.store(0, std::memory_order_release);
		tail.store(0, std::memory_order_release);
		nextSequence.store(0, std::memory_order_release);
		resetMetrics();
	}

	const QueueMetrics& getMetrics() const { return metrics; }
	void resetMetrics() {
		metrics.totalPushes.store(0, std::memory_order_release);
		metrics.totalPops.store(0, std::memory_order_release);
		metrics.pushFailures.store(0, std::memory_order_release);
		metrics.maxQueueSize.store(0, std::memory_order_release);
		metrics.totalLatencyUs.store(0, std::memory_order_release);
	}

private:
	std::array<Entry, QueueSize> buffer;
	std::atomic<size_t> head{0};
	std::atomic<size_t> tail{0};
	std::atomic<uint64_t> nextSequence;
	QueueMetrics metrics;
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_LOCKFREEEVENTQUEUE_H