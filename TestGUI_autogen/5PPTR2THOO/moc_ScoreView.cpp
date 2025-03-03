/****************************************************************************
** Meta object code from reading C++ file 'ScoreView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../include/presentation/ScoreView.h"
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
    uint offsetsAndSizes[34];
    char stringdata0[38];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[9];
    char stringdata5[9];
    char stringdata6[17];
    char stringdata7[10];
    char stringdata8[15];
    char stringdata9[13];
    char stringdata10[6];
    char stringdata11[6];
    char stringdata12[12];
    char stringdata13[5];
    char stringdata14[5];
    char stringdata15[23];
    char stringdata16[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__ScoreView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__ScoreView_t qt_meta_stringdata_MusicTrainer__presentation__ScoreView = {
    {
        QT_MOC_LITERAL(0, 37),  // "MusicTrainer::presentation::S..."
        QT_MOC_LITERAL(38, 9),  // "noteAdded"
        QT_MOC_LITERAL(48, 0),  // ""
        QT_MOC_LITERAL(49, 5),  // "pitch"
        QT_MOC_LITERAL(55, 8),  // "duration"
        QT_MOC_LITERAL(64, 8),  // "position"
        QT_MOC_LITERAL(73, 16),  // "viewportExpanded"
        QT_MOC_LITERAL(90, 9),  // "newBounds"
        QT_MOC_LITERAL(100, 14),  // "onScoreChanged"
        QT_MOC_LITERAL(115, 12),  // "onVoiceAdded"
        QT_MOC_LITERAL(128, 5),  // "Voice"
        QT_MOC_LITERAL(134, 5),  // "voice"
        QT_MOC_LITERAL(140, 11),  // "onNoteAdded"
        QT_MOC_LITERAL(152, 4),  // "Note"
        QT_MOC_LITERAL(157, 4),  // "note"
        QT_MOC_LITERAL(162, 22),  // "checkViewportExpansion"
        QT_MOC_LITERAL(185, 15)   // "handleNoteAdded"
    },
    "MusicTrainer::presentation::ScoreView",
    "noteAdded",
    "",
    "pitch",
    "duration",
    "position",
    "viewportExpanded",
    "newBounds",
    "onScoreChanged",
    "onVoiceAdded",
    "Voice",
    "voice",
    "onNoteAdded",
    "Note",
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
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   56,    2, 0x06,    1 /* Public */,
       6,    1,   63,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    0,   66,    2, 0x08,    7 /* Private */,
       9,    1,   67,    2, 0x08,    8 /* Private */,
      12,    1,   70,    2, 0x08,   10 /* Private */,
      15,    0,   73,    2, 0x08,   12 /* Private */,
      16,    3,   74,    2, 0x08,   13 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::QRectF,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Int,    3,    4,    5,

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
        // method 'noteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'viewportExpanded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRectF &, std::false_type>,
        // method 'onScoreChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onVoiceAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Voice &, std::false_type>,
        // method 'onNoteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Note &, std::false_type>,
        // method 'checkViewportExpansion'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleNoteAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::ScoreView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScoreView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->noteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 1: _t->viewportExpanded((*reinterpret_cast< std::add_pointer_t<QRectF>>(_a[1]))); break;
        case 2: _t->onScoreChanged(); break;
        case 3: _t->onVoiceAdded((*reinterpret_cast< std::add_pointer_t<Voice>>(_a[1]))); break;
        case 4: _t->onNoteAdded((*reinterpret_cast< std::add_pointer_t<Note>>(_a[1]))); break;
        case 5: _t->checkViewportExpansion(); break;
        case 6: _t->handleNoteAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ScoreView::*)(int , double , int );
            if (_t _q_method = &ScoreView::noteAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
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
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void MusicTrainer::presentation::ScoreView::noteAdded(int _t1, double _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MusicTrainer::presentation::ScoreView::viewportExpanded(const QRectF & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
