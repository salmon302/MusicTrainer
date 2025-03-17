#include "domain/music/Note.h"
#include <memory>

namespace MusicTrainer {
namespace music {

class Note::NoteImpl {
public:
    Pitch pitch;
    double duration;
    int position;
    bool rest;

    NoteImpl() : duration(0), position(0), rest(false) {}
    NoteImpl(const Pitch& p, double d, int pos) 
        : pitch(p), duration(d), position(pos), rest(false) {}
};

Note::Note() : m_impl(std::make_unique<NoteImpl>()) {}

Note::Note(const Pitch& pitch, double duration, int position)
    : m_impl(std::make_unique<NoteImpl>(pitch, duration, position)) {}

Note::Note(const Note& other)
    : m_impl(std::make_unique<NoteImpl>(*other.m_impl)) {}

Note::Note(Note&& other) noexcept = default;

Note& Note::operator=(const Note& other) {
    if (this != &other) {
        m_impl = std::make_unique<NoteImpl>(*other.m_impl);
    }
    return *this;
}

Note& Note::operator=(Note&& other) noexcept = default;

Note::~Note() = default;

const Pitch& Note::getPitch() const {
    return m_impl->pitch;
}

double Note::getDuration() const {
    return m_impl->duration;
}

int Note::getPosition() const {
    return m_impl->position;
}

void Note::setPosition(int pos) {
    m_impl->position = pos;
}

bool Note::isRest() const {
    return m_impl->rest;
}

} // namespace music
} // namespace MusicTrainer