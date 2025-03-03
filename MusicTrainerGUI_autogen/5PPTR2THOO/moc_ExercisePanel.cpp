/****************************************************************************
** Meta object code from reading C++ file 'ExercisePanel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../include/presentation/ExercisePanel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ExercisePanel.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel_t {
    uint offsetsAndSizes[22];
    char stringdata0[42];
    char stringdata1[23];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[6];
    char stringdata5[16];
    char stringdata6[13];
    char stringdata7[23];
    char stringdata8[14];
    char stringdata9[24];
    char stringdata10[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel_t qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel = {
    {
        QT_MOC_LITERAL(0, 41),  // "MusicTrainer::presentation::E..."
        QT_MOC_LITERAL(42, 22),  // "checkSolutionRequested"
        QT_MOC_LITERAL(65, 0),  // ""
        QT_MOC_LITERAL(66, 13),  // "hintRequested"
        QT_MOC_LITERAL(80, 5),  // "level"
        QT_MOC_LITERAL(86, 15),  // "exerciseChanged"
        QT_MOC_LITERAL(102, 12),  // "exerciseType"
        QT_MOC_LITERAL(115, 22),  // "onCheckSolutionClicked"
        QT_MOC_LITERAL(138, 13),  // "onHintClicked"
        QT_MOC_LITERAL(152, 23),  // "onRuleViolationSelected"
        QT_MOC_LITERAL(176, 5)   // "index"
    },
    "MusicTrainer::presentation::ExercisePanel",
    "checkSolutionRequested",
    "",
    "hintRequested",
    "level",
    "exerciseChanged",
    "exerciseType",
    "onCheckSolutionClicked",
    "onHintClicked",
    "onRuleViolationSelected",
    "index"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__ExercisePanel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   50,    2, 0x06,    1 /* Public */,
       3,    1,   51,    2, 0x06,    2 /* Public */,
       5,    1,   54,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,   57,    2, 0x08,    6 /* Private */,
       8,    0,   58,    2, 0x08,    7 /* Private */,
       9,    1,   59,    2, 0x08,    8 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QString,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   10,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::ExercisePanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__ExercisePanel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ExercisePanel, std::true_type>,
        // method 'checkSolutionRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'hintRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'exerciseChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onCheckSolutionClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onHintClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRuleViolationSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::ExercisePanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ExercisePanel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->checkSolutionRequested(); break;
        case 1: _t->hintRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->exerciseChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onCheckSolutionClicked(); break;
        case 4: _t->onHintClicked(); break;
        case 5: _t->onRuleViolationSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ExercisePanel::*)();
            if (_t _q_method = &ExercisePanel::checkSolutionRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ExercisePanel::*)(int );
            if (_t _q_method = &ExercisePanel::hintRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ExercisePanel::*)(const QString & );
            if (_t _q_method = &ExercisePanel::exerciseChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *MusicTrainer::presentation::ExercisePanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::ExercisePanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__ExercisePanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MusicTrainer::presentation::ExercisePanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void MusicTrainer::presentation::ExercisePanel::checkSolutionRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MusicTrainer::presentation::ExercisePanel::hintRequested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MusicTrainer::presentation::ExercisePanel::exerciseChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
