/****************************************************************************
** Meta object code from reading C++ file 'ScoreViewModel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/presentation/viewmodels/ScoreViewModel.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScoreViewModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel_t {
    uint offsetsAndSizes[26];
    char stringdata0[55];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[11];
    char stringdata5[12];
    char stringdata6[5];
    char stringdata7[12];
    char stringdata8[9];
    char stringdata9[11];
    char stringdata10[13];
    char stringdata11[6];
    char stringdata12[13];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel_t qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel = {
    {
        QT_MOC_LITERAL(0, 54),  // "MusicTrainer::presentation::v..."
        QT_MOC_LITERAL(55, 12),  // "scoreChanged"
        QT_MOC_LITERAL(68, 0),  // ""
        QT_MOC_LITERAL(69, 9),  // "noteAdded"
        QT_MOC_LITERAL(79, 10),  // "voiceIndex"
        QT_MOC_LITERAL(90, 11),  // "music::Note"
        QT_MOC_LITERAL(102, 4),  // "note"
        QT_MOC_LITERAL(107, 11),  // "noteRemoved"
        QT_MOC_LITERAL(119, 8),  // "position"
        QT_MOC_LITERAL(128, 10),  // "voiceAdded"
        QT_MOC_LITERAL(139, 12),  // "music::Voice"
        QT_MOC_LITERAL(152, 5),  // "voice"
        QT_MOC_LITERAL(158, 12)   // "voiceRemoved"
    },
    "MusicTrainer::presentation::viewmodels::ScoreViewModel",
    "scoreChanged",
    "",
    "noteAdded",
    "voiceIndex",
    "music::Note",
    "note",
    "noteRemoved",
    "position",
    "voiceAdded",
    "music::Voice",
    "voice",
    "voiceRemoved"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__viewmodels__ScoreViewModel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    1 /* Public */,
       3,    2,   45,    2, 0x06,    2 /* Public */,
       7,    2,   50,    2, 0x06,    5 /* Public */,
       9,    1,   55,    2, 0x06,    8 /* Public */,
      12,    1,   58,    2, 0x06,   10 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 5,    4,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    4,    8,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, QMetaType::Int,    4,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::viewmodels::ScoreViewModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__viewmodels__ScoreViewModel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ScoreViewModel, std::true_type>,
        // method 'scoreChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'noteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const music::Note &, std::false_type>,
        // method 'noteRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'voiceAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const music::Voice &, std::false_type>,
        // method 'voiceRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::viewmodels::ScoreViewModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScoreViewModel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->scoreChanged(); break;
        case 1: _t->noteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<music::Note>>(_a[2]))); break;
        case 2: _t->noteRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->voiceAdded((*reinterpret_cast< std::add_pointer_t<music::Voice>>(_a[1]))); break;
        case 4: _t->voiceRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ScoreViewModel::*)();
            if (_t _q_method = &ScoreViewModel::scoreChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ScoreViewModel::*)(int , const music::Note & );
            if (_t _q_method = &ScoreViewModel::noteAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ScoreViewModel::*)(int , int );
            if (_t _q_method = &ScoreViewModel::noteRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ScoreViewModel::*)(const music::Voice & );
            if (_t _q_method = &ScoreViewModel::voiceAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ScoreViewModel::*)(int );
            if (_t _q_method = &ScoreViewModel::voiceRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *MusicTrainer::presentation::viewmodels::ScoreViewModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::viewmodels::ScoreViewModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__viewmodels__ScoreViewModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int MusicTrainer::presentation::viewmodels::ScoreViewModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void MusicTrainer::presentation::viewmodels::ScoreViewModel::scoreChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MusicTrainer::presentation::viewmodels::ScoreViewModel::noteAdded(int _t1, const music::Note & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MusicTrainer::presentation::viewmodels::ScoreViewModel::noteRemoved(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MusicTrainer::presentation::viewmodels::ScoreViewModel::voiceAdded(const music::Voice & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void MusicTrainer::presentation::viewmodels::ScoreViewModel::voiceRemoved(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
