#pragma once

#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include <memory>

namespace MusicTrainer {
namespace music {

class Note {
public:
    Note();
    Note(const Pitch& pitch, double duration, int position);
    Note(const Note& other);
    Note(Note&& other) noexcept;
    Note& operator=(const Note& other);
    Note& operator=(Note&& other) noexcept;
    ~Note();

    const Pitch& getPitch() const;
    double getDuration() const;
    int getPosition() const;
    void setPosition(int pos);

private:
    class NoteImpl;
    std::unique_ptr<NoteImpl> m_impl;
};

} // namespace music
} // namespace MusicTrainer