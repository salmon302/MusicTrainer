/****************************************************************************
** Meta object code from reading C++ file 'FeedbackArea.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/presentation/FeedbackArea.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FeedbackArea.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea_t {
    uint offsetsAndSizes[26];
    char stringdata0[41];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[11];
    char stringdata5[13];
    char stringdata6[6];
    char stringdata7[27];
    char stringdata8[4];
    char stringdata9[7];
    char stringdata10[27];
    char stringdata11[17];
    char stringdata12[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea_t qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea = {
    {
        QT_MOC_LITERAL(0, 40),  // "MusicTrainer::presentation::F..."
        QT_MOC_LITERAL(41, 20),  // "feedbackItemSelected"
        QT_MOC_LITERAL(62, 0),  // ""
        QT_MOC_LITERAL(63, 8),  // "position"
        QT_MOC_LITERAL(72, 10),  // "voiceIndex"
        QT_MOC_LITERAL(83, 12),  // "onTabChanged"
        QT_MOC_LITERAL(96, 5),  // "index"
        QT_MOC_LITERAL(102, 26),  // "onVoiceAnalysisItemClicked"
        QT_MOC_LITERAL(129, 3),  // "row"
        QT_MOC_LITERAL(133, 6),  // "column"
        QT_MOC_LITERAL(140, 26),  // "onValidationMessageClicked"
        QT_MOC_LITERAL(167, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(184, 4)   // "item"
    },
    "MusicTrainer::presentation::FeedbackArea",
    "feedbackItemSelected",
    "",
    "position",
    "voiceIndex",
    "onTabChanged",
    "index",
    "onVoiceAnalysisItemClicked",
    "row",
    "column",
    "onValidationMessageClicked",
    "QListWidgetItem*",
    "item"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__FeedbackArea[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   38,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    1,   43,    2, 0x08,    4 /* Private */,
       7,    2,   46,    2, 0x08,    6 /* Private */,
      10,    1,   51,    2, 0x08,    9 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::FeedbackArea::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__FeedbackArea,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FeedbackArea, std::true_type>,
        // method 'feedbackItemSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onVoiceAnalysisItemClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onValidationMessageClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::FeedbackArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FeedbackArea *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->feedbackItemSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->onTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onVoiceAnalysisItemClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->onValidationMessageClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FeedbackArea::*)(int , int );
            if (_t _q_method = &FeedbackArea::feedbackItemSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *MusicTrainer::presentation::FeedbackArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::FeedbackArea::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__FeedbackArea.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MusicTrainer::presentation::FeedbackArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void MusicTrainer::presentation::FeedbackArea::feedbackItemSelected(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
