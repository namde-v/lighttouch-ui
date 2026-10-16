/****************************************************************************
** Meta object code from reading C++ file 'LTDebugPortAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTDebugPortAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTDebugPortAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTDebugPortAdapter_t {
    uint offsetsAndSizes[22];
    char stringdata0[19];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[8];
    char stringdata5[11];
    char stringdata6[8];
    char stringdata7[12];
    char stringdata8[8];
    char stringdata9[11];
    char stringdata10[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTDebugPortAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTDebugPortAdapter_t qt_meta_stringdata_LTDebugPortAdapter = {
    {
        QT_MOC_LITERAL(0, 18),  // "LTDebugPortAdapter"
        QT_MOC_LITERAL(19, 17),  // "deviceUuidChanged"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 12),  // "stateChanged"
        QT_MOC_LITERAL(51, 7),  // "refresh"
        QT_MOC_LITERAL(59, 10),  // "setEnabled"
        QT_MOC_LITERAL(70, 7),  // "enabled"
        QT_MOC_LITERAL(78, 11),  // "copyCommand"
        QT_MOC_LITERAL(90, 7),  // "command"
        QT_MOC_LITERAL(98, 10),  // "deviceUuid"
        QT_MOC_LITERAL(109, 5)   // "state"
    },
    "LTDebugPortAdapter",
    "deviceUuidChanged",
    "",
    "stateChanged",
    "refresh",
    "setEnabled",
    "enabled",
    "copyCommand",
    "command",
    "deviceUuid",
    "state"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTDebugPortAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       2,   53, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    3 /* Public */,
       3,    0,   45,    2, 0x06,    4 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,   46,    2, 0x02,    5 /* Public */,
       5,    1,   47,    2, 0x02,    6 /* Public */,
       7,    1,   50,    2, 0x02,    8 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::QString,    8,

 // properties: name, type, flags
       9, QMetaType::QString, 0x00015103, uint(0), 0,
      10, QMetaType::QVariantMap, 0x00015001, uint(1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTDebugPortAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTDebugPortAdapter.offsetsAndSizes,
    qt_meta_data_LTDebugPortAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTDebugPortAdapter_t,
        // property 'deviceUuid'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'state'
        QtPrivate::TypeAndForceComplete<QVariantMap, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTDebugPortAdapter, std::true_type>,
        // method 'deviceUuidChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'copyCommand'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LTDebugPortAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTDebugPortAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceUuidChanged(); break;
        case 1: _t->stateChanged(); break;
        case 2: _t->refresh(); break;
        case 3: _t->setEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->copyCommand((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTDebugPortAdapter::*)();
            if (_t _q_method = &LTDebugPortAdapter::deviceUuidChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTDebugPortAdapter::*)();
            if (_t _q_method = &LTDebugPortAdapter::stateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTDebugPortAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->deviceUuid(); break;
        case 1: *reinterpret_cast< QVariantMap*>(_v) = _t->state(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTDebugPortAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setDeviceUuid(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *LTDebugPortAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTDebugPortAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTDebugPortAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTDebugPortAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
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
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void LTDebugPortAdapter::deviceUuidChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTDebugPortAdapter::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
