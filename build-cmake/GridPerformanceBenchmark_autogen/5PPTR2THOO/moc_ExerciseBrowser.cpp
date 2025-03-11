/****************************************************************************
** Meta object code from reading C++ file 'ExerciseBrowser.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/presentation/ExerciseBrowser.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ExerciseBrowser.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser_t {
    uint offsetsAndSizes[26];
    char stringdata0[44];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[45];
    char stringdata4[9];
    char stringdata5[20];
    char stringdata6[6];
    char stringdata7[18];
    char stringdata8[19];
    char stringdata9[17];
    char stringdata10[5];
    char stringdata11[30];
    char stringdata12[24];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser_t qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser = {
    {
        QT_MOC_LITERAL(0, 43),  // "MusicTrainer::presentation::E..."
        QT_MOC_LITERAL(44, 16),  // "exerciseSelected"
        QT_MOC_LITERAL(61, 0),  // ""
        QT_MOC_LITERAL(62, 44),  // "std::shared_ptr<domain::exerc..."
        QT_MOC_LITERAL(107, 8),  // "exercise"
        QT_MOC_LITERAL(116, 19),  // "onDifficultyChanged"
        QT_MOC_LITERAL(136, 5),  // "index"
        QT_MOC_LITERAL(142, 17),  // "onCategoryChanged"
        QT_MOC_LITERAL(160, 18),  // "onExerciseSelected"
        QT_MOC_LITERAL(179, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(196, 4),  // "item"
        QT_MOC_LITERAL(201, 29),  // "onCreateCustomExerciseClicked"
        QT_MOC_LITERAL(231, 23)   // "onSelectExerciseClicked"
    },
    "MusicTrainer::presentation::ExerciseBrowser",
    "exerciseSelected",
    "",
    "std::shared_ptr<domain::exercises::Exercise>",
    "exercise",
    "onDifficultyChanged",
    "index",
    "onCategoryChanged",
    "onExerciseSelected",
    "QListWidgetItem*",
    "item",
    "onCreateCustomExerciseClicked",
    "onSelectExerciseClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__ExerciseBrowser[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    1,   53,    2, 0x08,    3 /* Private */,
       7,    1,   56,    2, 0x08,    5 /* Private */,
       8,    1,   59,    2, 0x08,    7 /* Private */,
      11,    0,   62,    2, 0x08,    9 /* Private */,
      12,    0,   63,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::ExerciseBrowser::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__ExerciseBrowser,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ExerciseBrowser, std::true_type>,
        // method 'exerciseSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::shared_ptr<domain::exercises::Exercise>, std::false_type>,
        // method 'onDifficultyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onCategoryChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onExerciseSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onCreateCustomExerciseClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSelectExerciseClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::ExerciseBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ExerciseBrowser *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->exerciseSelected((*reinterpret_cast< std::add_pointer_t<std::shared_ptr<domain::exercises::Exercise>>>(_a[1]))); break;
        case 1: _t->onDifficultyChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onCategoryChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->onExerciseSelected((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 4: _t->onCreateCustomExerciseClicked(); break;
        case 5: _t->onSelectExerciseClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ExerciseBrowser::*)(std::shared_ptr<domain::exercises::Exercise> );
            if (_t _q_method = &ExerciseBrowser::exerciseSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *MusicTrainer::presentation::ExerciseBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::ExerciseBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__ExerciseBrowser.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MusicTrainer::presentation::ExerciseBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void MusicTrainer::presentation::ExerciseBrowser::exerciseSelected(std::shared_ptr<domain::exercises::Exercise> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
