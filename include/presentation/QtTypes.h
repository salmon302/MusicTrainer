#pragma once

#include <QtCore/QMetaType>
#include "domain/music/Voice.h"
#include "domain/music/Note.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

// Define metatypes for our domain classes
Q_DECLARE_METATYPE(MusicTrainer::music::Voice);
Q_DECLARE_METATYPE(MusicTrainer::music::Note);

namespace MusicTrainer::presentation {

inline void registerQtTypes() {
    qRegisterMetaType<MusicTrainer::music::Voice>("MusicTrainer::music::Voice");
    qRegisterMetaType<MusicTrainer::music::Note>("MusicTrainer::music::Note");
}

} // namespace MusicTrainer::presentation