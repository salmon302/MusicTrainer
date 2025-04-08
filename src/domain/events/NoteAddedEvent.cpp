#include "domain/events/NoteAddedEvent.h"
#include <sstream>
#include <iomanip>

namespace MusicTrainer::music::events {

std::unique_ptr<NoteAddedEvent> NoteAddedEvent::create(
    size_t voiceIndex,
    const MusicTrainer::music::Pitch& pitch,
    const MusicTrainer::music::Duration& duration,
    const MusicTrainer::music::Voice::TimeSignature& timeSignature,
    const std::string& correlationId)
{
    return std::unique_ptr<NoteAddedEvent>(
        new NoteAddedEvent(voiceIndex, pitch, duration, timeSignature, correlationId)
    );
}

NoteAddedEvent::NoteAddedEvent(
    size_t voiceIndex,
    const MusicTrainer::music::Pitch& pitch,
    const MusicTrainer::music::Duration& duration,
    const MusicTrainer::music::Voice::TimeSignature& timeSignature,
    const std::string& correlationId)
    : voiceIndex(voiceIndex)
    , pitch(pitch)
    , duration(duration)
    , timeSignature(timeSignature)
{
    Event::setCorrelationId(correlationId);
}

NoteAddedEvent::NoteAddedEvent(const NoteAddedEvent& other)
    : Event(other)
    , timestamp(other.timestamp.load(std::memory_order_acquire))
    , voiceIndex(other.voiceIndex.load(std::memory_order_acquire))
    , pitch(other.pitch)
    , duration(other.duration)
    , timeSignature(other.timeSignature)
{
}

NoteAddedEvent& NoteAddedEvent::operator=(const NoteAddedEvent& other) {
    if (this != &other) {
        Event::operator=(other);
        timestamp.store(
            other.timestamp.load(std::memory_order_acquire),
            std::memory_order_release
        );
        voiceIndex.store(
            other.voiceIndex.load(std::memory_order_acquire),
            std::memory_order_release
        );
        const_cast<MusicTrainer::music::Pitch&>(pitch) = other.pitch;
        const_cast<MusicTrainer::music::Duration&>(duration) = other.duration;
        const_cast<MusicTrainer::music::Voice::TimeSignature&>(timeSignature) = other.timeSignature;
    }
    return *this;
}

void NoteAddedEvent::apply(MusicTrainer::music::Score& score) const {
    auto* voice = score.getVoice(voiceIndex.load(std::memory_order_acquire));
    if (!voice) return;

    voice->addNote(pitch, duration.getTotalBeats());
}

std::string NoteAddedEvent::getDescription() const {
    std::stringstream ss;
    ss << "Added " << pitch.toString() << " ("
       << duration.toString() << ") to voice " 
       << voiceIndex.load(std::memory_order_acquire);
    return ss.str();
}

std::unique_ptr<Event> NoteAddedEvent::clone() const {
    return std::unique_ptr<Event>(new NoteAddedEvent(*this));
}

} // namespace MusicTrainer::music::events
