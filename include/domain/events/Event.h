#pragma once

#include "domain/music/Score.h"
#include <string>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <atomic>

namespace MusicTrainer::music::events {

class Event {
public:
    Event() = default;
    
    Event(const Event& other) {
        correlationId = other.correlationId;
        metadata = other.metadata;
    }
    
    Event& operator=(const Event& other) {
        if (this != &other) {
            correlationId = other.correlationId;
            metadata = other.metadata;
        }
        return *this;
    }
    
    virtual ~Event() = default;
    
    virtual std::string getType() const = 0;
    virtual void apply(MusicTrainer::music::Score& target) const = 0;
    virtual std::chrono::system_clock::time_point getTimestamp() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::unique_ptr<Event> clone() const = 0;
    
    std::string getCorrelationId() const {
        return correlationId;
    }
    
    void setCorrelationId(const std::string& id) {
        correlationId = id;
    }
    
    std::unordered_map<std::string, std::string> getAllMetadata() const {
        return metadata;
    }
protected:
    std::string correlationId;
    std::unordered_map<std::string, std::string> metadata;
};

class EmptyEvent : public Event {
public:
    std::string getType() const override { return "Empty"; }
    void apply(MusicTrainer::music::Score& target) const override {}
    std::chrono::system_clock::time_point getTimestamp() const override { return std::chrono::system_clock::time_point{}; }
    std::string getDescription() const override { return "Empty Event"; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<EmptyEvent>(); }
};

} // namespace MusicTrainer::music::events


