/****************************************************************************
** Meta object code from reading C++ file 'GridManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/presentation/grid/GridManager.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GridManager.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager_t {
    uint offsetsAndSizes[16];
    char stringdata0[46];
    char stringdata1[12];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[14];
    char stringdata5[7];
    char stringdata6[13];
    char stringdata7[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager_t qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager = {
    {
        QT_MOC_LITERAL(0, 45),  // "MusicTrainer::presentation::g..."
        QT_MOC_LITERAL(46, 11),  // "gridChanged"
        QT_MOC_LITERAL(58, 0),  // ""
        QT_MOC_LITERAL(59, 15),  // "viewportChanged"
        QT_MOC_LITERAL(75, 13),  // "memoryWarning"
        QT_MOC_LITERAL(89, 6),  // "size_t"
        QT_MOC_LITERAL(96, 12),  // "currentUsage"
        QT_MOC_LITERAL(109, 5)   // "limit"
    },
    "MusicTrainer::presentation::grid::GridManager",
    "gridChanged",
    "",
    "viewportChanged",
    "memoryWarning",
    "size_t",
    "currentUsage",
    "limit"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__grid__GridManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x06,    1 /* Public */,
       3,    0,   33,    2, 0x06,    2 /* Public */,
       4,    2,   34,    2, 0x06,    3 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5,    6,    7,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::grid::GridManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__grid__GridManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GridManager, std::true_type>,
        // method 'gridChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'viewportChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'memoryWarning'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<size_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<size_t, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::grid::GridManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GridManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->gridChanged(); break;
        case 1: _t->viewportChanged(); break;
        case 2: _t->memoryWarning((*reinterpret_cast< std::add_pointer_t<size_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<size_t>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GridManager::*)();
            if (_t _q_method = &GridManager::gridChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GridManager::*)();
            if (_t _q_method = &GridManager::viewportChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GridManager::*)(size_t , size_t );
            if (_t _q_method = &GridManager::memoryWarning; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *MusicTrainer::presentation::grid::GridManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::grid::GridManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__grid__GridManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MusicTrainer::presentation::grid::GridManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void MusicTrainer::presentation::grid::GridManager::gridChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MusicTrainer::presentation::grid::GridManager::viewportChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MusicTrainer::presentation::grid::GridManager::memoryWarning(size_t _t1, size_t _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
