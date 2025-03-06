/****************************************************************************
** Meta object code from reading C++ file 'ScoreView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../include/presentation/ScoreView.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScoreView.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MusicTrainer__presentation__ScoreView_t {
    uint offsetsAndSizes[38];
    char stringdata0[38];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[11];
    char stringdata5[17];
    char stringdata6[10];
    char stringdata7[10];
    char stringdata8[6];
    char stringdata9[9];
    char stringdata10[15];
    char stringdata11[13];
    char stringdata12[27];
    char stringdata13[6];
    char stringdata14[12];
    char stringdata15[26];
    char stringdata16[5];
    char stringdata17[23];
    char stringdata18[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__ScoreView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__ScoreView_t qt_meta_stringdata_MusicTrainer__presentation__ScoreView = {
    {
        QT_MOC_LITERAL(0, 37),  // "MusicTrainer::presentation::S..."
        QT_MOC_LITERAL(38, 12),  // "noteSelected"
        QT_MOC_LITERAL(51, 0),  // ""
        QT_MOC_LITERAL(52, 8),  // "position"
        QT_MOC_LITERAL(61, 10),  // "voiceIndex"
        QT_MOC_LITERAL(72, 16),  // "viewportExpanded"
        QT_MOC_LITERAL(89, 9),  // "newBounds"
        QT_MOC_LITERAL(99, 9),  // "noteAdded"
        QT_MOC_LITERAL(109, 5),  // "pitch"
        QT_MOC_LITERAL(115, 8),  // "duration"
        QT_MOC_LITERAL(124, 14),  // "onScoreChanged"
        QT_MOC_LITERAL(139, 12),  // "onVoiceAdded"
        QT_MOC_LITERAL(152, 26),  // "MusicTrainer::music::Voice"
        QT_MOC_LITERAL(179, 5),  // "voice"
        QT_MOC_LITERAL(185, 11),  // "onNoteAdded"
        QT_MOC_LITERAL(197, 25),  // "MusicTrainer::music::Note"
        QT_MOC_LITERAL(223, 4),  // "note"
        QT_MOC_LITERAL(228, 22),  // "checkViewportExpansion"
        QT_MOC_LITERAL(251, 15)   // "handleNoteAdded"
    },
    "MusicTrainer::presentation::ScoreView",
    "noteSelected",
    "",
    "position",
    "voiceIndex",
    "viewportExpanded",
    "newBounds",
    "noteAdded",
    "pitch",
    "duration",
    "onScoreChanged",
    "onVoiceAdded",
    "MusicTrainer::music::Voice",
    "voice",
    "onNoteAdded",
    "MusicTrainer::music::Note",
    "note",
    "checkViewportExpansion",
    "handleNoteAdded"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__ScoreView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   74,    2, 0x06,    1 /* Public */,
       5,    1,   79,    2, 0x06,    4 /* Public */,
       7,    3,   82,    2, 0x06,    6 /* Public */,
       7,    2,   89,    2, 0x26,   10 /* Public | MethodCloned */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    0,   94,    2, 0x0a,   13 /* Public */,
      11,    1,   95,    2, 0x0a,   14 /* Public */,
      14,    1,   98,    2, 0x0a,   16 /* Public */,
      17,    0,  101,    2, 0x0a,   18 /* Public */,
      18,    3,  102,    2, 0x0a,   19 /* Public */,
      18,    2,  109,    2, 0x2a,   23 /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::QRectF,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Int,    8,    9,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    8,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Int,    8,    9,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    8,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::ScoreView::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsView::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__ScoreView.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__ScoreView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__ScoreView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ScoreView, std::true_type>,
        // method 'noteSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'viewportExpanded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRectF &, std::false_type>,
        // method 'noteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'noteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'onScoreChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onVoiceAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const MusicTrainer::music::Voice &, std::false_type>,
        // method 'onNoteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const MusicTrainer::music::Note &, std::false_type>,
        // method 'checkViewportExpansion'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleNoteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleNoteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::ScoreView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScoreView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->noteSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->viewportExpanded((*reinterpret_cast< std::add_pointer_t<QRectF>>(_a[1]))); break;
        case 2: _t->noteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 3: _t->noteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 4: _t->onScoreChanged(); break;
        case 5: _t->onVoiceAdded((*reinterpret_cast< std::add_pointer_t<MusicTrainer::music::Voice>>(_a[1]))); break;
        case 6: _t->onNoteAdded((*reinterpret_cast< std::add_pointer_t<MusicTrainer::music::Note>>(_a[1]))); break;
        case 7: _t->checkViewportExpansion(); break;
        case 8: _t->handleNoteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 9: _t->handleNoteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ScoreView::*)(int , int );
            if (_t _q_method = &ScoreView::noteSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ScoreView::*)(const QRectF & );
            if (_t _q_method = &ScoreView::viewportExpanded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ScoreView::*)(int , double , int );
            if (_t _q_method = &ScoreView::noteAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *MusicTrainer::presentation::ScoreView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::ScoreView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__ScoreView.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsView::qt_metacast(_clname);
}

int MusicTrainer::presentation::ScoreView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void MusicTrainer::presentation::ScoreView::noteSelected(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MusicTrainer::presentation::ScoreView::viewportExpanded(const QRectF & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MusicTrainer::presentation::ScoreView::noteAdded(int _t1, double _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
