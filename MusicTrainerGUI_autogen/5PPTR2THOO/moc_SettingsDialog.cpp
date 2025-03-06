/****************************************************************************
** Meta object code from reading C++ file 'SettingsDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../include/presentation/SettingsDialog.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SettingsDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog_t {
    uint offsetsAndSizes[26];
    char stringdata0[43];
    char stringdata1[25];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[26];
    char stringdata5[28];
    char stringdata6[15];
    char stringdata7[26];
    char stringdata8[14];
    char stringdata9[8];
    char stringdata10[14];
    char stringdata11[16];
    char stringdata12[25];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog_t qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog = {
    {
        QT_MOC_LITERAL(0, 42),  // "MusicTrainer::presentation::S..."
        QT_MOC_LITERAL(43, 24),  // "onMidiInputDeviceChanged"
        QT_MOC_LITERAL(68, 0),  // ""
        QT_MOC_LITERAL(69, 5),  // "index"
        QT_MOC_LITERAL(75, 25),  // "onMidiOutputDeviceChanged"
        QT_MOC_LITERAL(101, 27),  // "onRefreshMidiDevicesClicked"
        QT_MOC_LITERAL(129, 14),  // "onThemeChanged"
        QT_MOC_LITERAL(144, 25),  // "onRuleSetSelectionChanged"
        QT_MOC_LITERAL(170, 13),  // "onRuleToggled"
        QT_MOC_LITERAL(184, 7),  // "checked"
        QT_MOC_LITERAL(192, 13),  // "onSaveClicked"
        QT_MOC_LITERAL(206, 15),  // "onCancelClicked"
        QT_MOC_LITERAL(222, 24)   // "onRestoreDefaultsClicked"
    },
    "MusicTrainer::presentation::SettingsDialog",
    "onMidiInputDeviceChanged",
    "",
    "index",
    "onMidiOutputDeviceChanged",
    "onRefreshMidiDevicesClicked",
    "onThemeChanged",
    "onRuleSetSelectionChanged",
    "onRuleToggled",
    "checked",
    "onSaveClicked",
    "onCancelClicked",
    "onRestoreDefaultsClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MusicTrainer__presentation__SettingsDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   68,    2, 0x08,    1 /* Private */,
       4,    1,   71,    2, 0x08,    3 /* Private */,
       5,    0,   74,    2, 0x08,    5 /* Private */,
       6,    1,   75,    2, 0x08,    6 /* Private */,
       7,    0,   78,    2, 0x08,    8 /* Private */,
       8,    1,   79,    2, 0x08,    9 /* Private */,
      10,    0,   82,    2, 0x08,   11 /* Private */,
      11,    0,   83,    2, 0x08,   12 /* Private */,
      12,    0,   84,    2, 0x08,   13 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MusicTrainer::presentation::SettingsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog.offsetsAndSizes,
    qt_meta_data_MusicTrainer__presentation__SettingsDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SettingsDialog, std::true_type>,
        // method 'onMidiInputDeviceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onMidiOutputDeviceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onRefreshMidiDevicesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onThemeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onRuleSetSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRuleToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onSaveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRestoreDefaultsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MusicTrainer::presentation::SettingsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SettingsDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onMidiInputDeviceChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->onMidiOutputDeviceChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onRefreshMidiDevicesClicked(); break;
        case 3: _t->onThemeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onRuleSetSelectionChanged(); break;
        case 5: _t->onRuleToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->onSaveClicked(); break;
        case 7: _t->onCancelClicked(); break;
        case 8: _t->onRestoreDefaultsClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *MusicTrainer::presentation::SettingsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicTrainer::presentation::SettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MusicTrainer__presentation__SettingsDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MusicTrainer::presentation::SettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
