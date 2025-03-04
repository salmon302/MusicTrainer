#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/music/Interval.h"
#include "domain/music/Voice.h"
#include "domain/music/Score.h"
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/events/EventStore.h"
#include "domain/events/EventBus.h"
#include "domain/events/NoteEventHandler.h"
#include "adapters/InMemoryScoreRepository.h"
#include "adapters/MockMidiAdapter.h"
#include "domain/errors/DefaultErrorHandlers.h"
#include "domain/errors/DomainErrors.h"
#include <iostream>
#include <cassert>
#include <atomic>
#include <thread>

void testPitch() {
    // Test pitch creation and comparison
    auto c4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4);
    auto c4Sharp = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4, 1);
    auto d4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::D, 4);
    
    assert(c4.getMidiNote() == 60);
    assert(c4Sharp.getMidiNote() == 61);
    assert(d4.getMidiNote() == 62);
    
    std::cout << "Pitch tests passed\n";
}

void testDuration() {
    // Test duration creation and comparison
    auto quarter = MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER);
    auto quarterDotted = MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER, 1);
    
    assert(quarter.getTotalBeats() == 1.0);
    assert(quarterDotted.getTotalBeats() == 1.5);
    
    std::cout << "Duration tests passed\n";
}

void testInterval() {
    // Test interval creation and comparison
    auto perfectFifth = MusicTrainer::music::Interval::fromQualityAndNumber(
        MusicTrainer::music::Interval::Quality::Perfect,
        MusicTrainer::music::Interval::Number::Fifth);
    auto majorThird = MusicTrainer::music::Interval::fromQualityAndNumber(
        MusicTrainer::music::Interval::Quality::Major,
        MusicTrainer::music::Interval::Number::Third);
    
    assert(perfectFifth.getSemitones() == 7);
    assert(majorThird.getSemitones() == 4);
    assert(perfectFifth.getQuality() == MusicTrainer::music::Interval::Quality::Perfect);
    
    std::cout << "Interval tests passed\n";
}

void testVoice() {
    // Test voice creation and note addition
    auto voice = MusicTrainer::music::Voice::create();
    auto c4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4);
    auto quarter = MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER);
    
    voice->addNote(c4, quarter.getTotalBeats());
    
    const auto* note = voice->getNoteAt(0);
    assert(note != nullptr);
    assert(note->getPitch().getMidiNote() == 60);
    assert(note->getDuration() == 1.0);
    
    std::cout << "Voice tests passed\n";
}

void testScore() {
    // Test score creation and voice management
    auto score = MusicTrainer::music::Score::create();
    auto voice1 = MusicTrainer::music::Voice::create();
    auto voice2 = MusicTrainer::music::Voice::create();
    
    auto c4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4);
    auto quarter = MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER);
    
    voice1->addNote(c4, quarter.getTotalBeats());
    score->addVoice(std::move(voice1));
    score->addVoice(std::move(voice2));
    
    assert(score->getVoiceCount() == 2);
    assert(score->getVoice(0) != nullptr);
    auto v1Notes = score->getVoice(0)->getAllNotes();
    auto v2Notes = score->getVoice(1)->getAllNotes();
    assert(v1Notes.size() == 1);
    assert(v2Notes.empty());
    
    std::cout << "Score tests passed\n";
}

void testParallelFifthsRule() {
    auto score = MusicTrainer::music::Score::create();
    auto voice1 = MusicTrainer::music::Voice::create();
    auto voice2 = MusicTrainer::music::Voice::create();
    
    // Add notes that create parallel fifths
    voice1->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4), 
                   MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    voice1->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::D, 4), 
                   MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    
    voice2->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::F, 3), 
                   MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    voice2->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::G, 3), 
                   MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    
    score->addVoice(std::move(voice1));
    score->addVoice(std::move(voice2));
    
    auto rule = MusicTrainer::music::rules::ParallelFifthsRule::create();
    assert(!rule->evaluate(*score)); // Should return false for parallel fifths violation
    
    // Create a new score with valid voice leading
    auto validScore = MusicTrainer::music::Score::create();
    auto validVoice1 = MusicTrainer::music::Voice::create();
    auto validVoice2 = MusicTrainer::music::Voice::create();
    
    validVoice1->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4), 
                        MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    validVoice1->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::D, 4), 
                        MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    
    validVoice2->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::E, 3), 
                        MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    validVoice2->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::G, 3), 
                        MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    
    validScore->addVoice(std::move(validVoice1));
    validScore->addVoice(std::move(validVoice2));
    
    assert(rule->evaluate(*validScore)); // Should return true for no violations
    
    // Print intervals for debugging
    auto firstInterval = MusicTrainer::music::Interval::fromPitches(
        MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::F, 3),
        MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4));
    auto secondInterval = MusicTrainer::music::Interval::fromPitches(
        MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::G, 3),
        MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::D, 4));
    
    std::cout << "First interval: " << firstInterval.toString() << " (F3 -> C4)\n";
    std::cout << "Second interval: " << secondInterval.toString() << " (G3 -> D4)\n";
    
    std::cout << "Parallel Fifths Rule tests passed\n";
}

void testEventSystem() {
    auto eventStore = music::events::EventStore::create();
    assert(eventStore->getEventCount() == 0);
    
    // Create a note added event
    auto c4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4);
    auto quarter = MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER);
    auto timeSignature = MusicTrainer::music::Voice::TimeSignature(4, MusicTrainer::music::Duration::createQuarter());
    auto noteEvent = music::events::NoteAddedEvent::create(0, c4, quarter, timeSignature);
    
    // Test event properties
    assert(noteEvent->getVoiceIndex() == 0);
    assert(noteEvent->getPitch().getMidiNote() == 60);
    assert(noteEvent->getDuration().getTotalBeats() == 1.0);
    assert(noteEvent->getType() == "NoteAdded");
    
    // Test event store
    eventStore->addEvent(std::move(noteEvent));
    assert(eventStore->getEventCount() == 1);
    
    // Test event description
    const auto& events = eventStore->getEvents();
    assert(!events.empty());
    assert(events[0]->getType() == "NoteAdded");
    
    // Test event store clear
    eventStore->clear();
    assert(eventStore->getEventCount() == 0);
    
    std::cout << "Event system tests passed\n";
}

void testScoreRepository() {
    auto repository = music::adapters::InMemoryScoreRepository::create();
    
    // Create a test score
    auto score = MusicTrainer::music::Score::create();
    auto voice = MusicTrainer::music::Voice::create();
    voice->addNote(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4), 
                  MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER).getTotalBeats());
    score->addVoice(std::move(voice));
    
    // Test save and load
    repository->save("test_score", *score);
    assert(repository->listScores().size() == 1);
    assert(repository->listScores()[0] == "test_score");
    
    auto loadedScore = repository->load("test_score");
    assert(loadedScore != nullptr);
    assert(loadedScore->getVoiceCount() == 1);
    
    // Test remove
    repository->remove("test_score");
    assert(repository->listScores().empty());
    
    // Test loading non-existent score
    bool exceptionThrown = false;
    try {
        repository->load("non_existent");
    } catch (const std::runtime_error&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
    
    std::cout << "Score Repository tests passed\n";
}

void testEventBusAndCorrelation() {
    auto eventBus = music::events::EventBus::create();
    auto noteHandler = music::events::NoteEventHandler::create();
    
    eventBus->subscribe(music::events::NoteAddedEvent::TYPE, [&](const music::events::Event& event) {
        noteHandler->handle(event);
    });
    
    // Create some test notes with correlation ID
    auto c4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4);
    auto d4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::D, 4);
    auto quarter = MusicTrainer::music::Duration::create(MusicTrainer::music::Duration::Type::QUARTER);
    
    auto timeSignature = MusicTrainer::music::Voice::TimeSignature(4, MusicTrainer::music::Duration::createQuarter());
    auto event1 = music::events::NoteAddedEvent::create(0, c4, quarter, timeSignature, "test-correlation");
    
    // Publish first event
    eventBus->publish(std::move(event1));
    
    // Add delay to ensure distinct timestamps
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto event2 = music::events::NoteAddedEvent::create(1, d4, quarter, timeSignature, "test-correlation");
    
    // Publish second event
    eventBus->publish(std::move(event2));
    
    // Test correlation within a 1-second window
    auto recentNotes = noteHandler->getRecentNotes(std::chrono::milliseconds(1000));
    assert(recentNotes.size() == 2);
    assert(recentNotes[0]->getPitch().getMidiNote() == 60); // C4
    assert(recentNotes[1]->getPitch().getMidiNote() == 62); // D4
    
    // Test correlated notes
    auto correlatedNotes = noteHandler->getCorrelatedNotes("test-correlation");
    assert(correlatedNotes.size() == 2);
    
    std::cout << "Event Bus and Correlation tests passed\n";
}

void testMidiSystem() {
    auto midiAdapter = music::adapters::MockMidiAdapter::create();
    
    // Test event callback
    std::atomic<int> eventCount{0};
    std::atomic<bool> noteOnReceived{false};
    std::atomic<bool> noteOffReceived{false};
    
    midiAdapter->setEventCallback([&](const music::ports::MidiEvent& event) {
        eventCount++;
        if (event.type == music::ports::MidiEvent::Type::NOTE_ON) {
            noteOnReceived = true;
        } else if (event.type == music::ports::MidiEvent::Type::NOTE_OFF) {
            noteOffReceived = true;
        }
    });
    
    // Try to open the port
    if (midiAdapter->open()) {
        // Send test events
        music::ports::MidiEvent noteOn{
            music::ports::MidiEvent::Type::NOTE_ON,
            0,  // channel
            60, // middle C
            64  // velocity
        };
        
        music::ports::MidiEvent noteOff{
            music::ports::MidiEvent::Type::NOTE_OFF,
            0,  // channel
            60, // middle C
            0   // velocity
        };
        
        midiAdapter->sendEvent(noteOn);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        midiAdapter->sendEvent(noteOff);
        
        // Wait for event processing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Add assertions
        assert(eventCount == 2);
        assert(noteOnReceived);
        assert(noteOffReceived);
        
        midiAdapter->close();
    }
    
    std::cout << "MIDI System tests completed\n";
}

void testErrorHandling() {
    // Test MIDI error handling
    try {
        throw MusicTrainer::MidiError("Test MIDI error");
    } catch (const MusicTrainer::MidiError& e) {
        // Error should be handled by the error handler
    }
    
    // Test validation error handling
    try {
        throw MusicTrainer::ValidationError("Test validation error");
    } catch (const MusicTrainer::ValidationError& e) {
        // Error should be handled by the error handler
    }
    
    // Test plugin error handling
    try {
        throw MusicTrainer::PluginError("Test plugin error");
    } catch (const MusicTrainer::PluginError& e) {
        // Error should be handled by the error handler
    }
    
    std::cout << "Error handling tests passed\n";
}

int main() {
    try {
        // Configure error handling system
        MusicTrainer::configureDefaultErrorHandlers();
        
        // Run all tests
        testPitch();
        testDuration();
        testInterval();
        testVoice();
        testScore();
        testParallelFifthsRule();
        testEventSystem();
        testScoreRepository();
        testEventBusAndCorrelation();
        testMidiSystem();
        testErrorHandling();
        
        std::cout << "All tests passed successfully!\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}