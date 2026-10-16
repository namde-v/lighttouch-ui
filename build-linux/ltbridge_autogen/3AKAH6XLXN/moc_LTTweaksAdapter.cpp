/****************************************************************************
** Meta object code from reading C++ file 'LTTweaksAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTTweaksAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTTweaksAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTTweaksAdapter_t {
    uint offsetsAndSizes[42];
    char stringdata0[16];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[18];
    char stringdata5[8];
    char stringdata6[9];
    char stringdata7[8];
    char stringdata8[3];
    char stringdata9[9];
    char stringdata10[6];
    char stringdata11[9];
    char stringdata12[6];
    char stringdata13[15];
    char stringdata14[8];
    char stringdata15[9];
    char stringdata16[11];
    char stringdata17[7];
    char stringdata18[8];
    char stringdata19[8];
    char stringdata20[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTTweaksAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTTweaksAdapter_t qt_meta_stringdata_LTTweaksAdapter = {
    {
        QT_MOC_LITERAL(0, 15),  // "LTTweaksAdapter"
        QT_MOC_LITERAL(16, 17),  // "deviceUuidChanged"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 13),  // "tweaksChanged"
        QT_MOC_LITERAL(49, 17),  // "hiddenAppsChanged"
        QT_MOC_LITERAL(67, 7),  // "refresh"
        QT_MOC_LITERAL(75, 8),  // "setTweak"
        QT_MOC_LITERAL(84, 7),  // "tweakId"
        QT_MOC_LITERAL(92, 2),  // "on"
        QT_MOC_LITERAL(95, 8),  // "setColor"
        QT_MOC_LITERAL(104, 5),  // "color"
        QT_MOC_LITERAL(110, 8),  // "setClock"
        QT_MOC_LITERAL(119, 5),  // "epoch"
        QT_MOC_LITERAL(125, 14),  // "listHiddenApps"
        QT_MOC_LITERAL(140, 7),  // "openApp"
        QT_MOC_LITERAL(148, 8),  // "bundleId"
        QT_MOC_LITERAL(157, 10),  // "deviceUuid"
        QT_MOC_LITERAL(168, 6),  // "tweaks"
        QT_MOC_LITERAL(175, 7),  // "running"
        QT_MOC_LITERAL(183, 7),  // "message"
        QT_MOC_LITERAL(191, 10)   // "hiddenApps"
    },
    "LTTweaksAdapter",
    "deviceUuidChanged",
    "",
    "tweaksChanged",
    "hiddenAppsChanged",
    "refresh",
    "setTweak",
    "tweakId",
    "on",
    "setColor",
    "color",
    "setClock",
    "epoch",
    "listHiddenApps",
    "openApp",
    "bundleId",
    "deviceUuid",
    "tweaks",
    "running",
    "message",
    "hiddenApps"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTTweaksAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       5,   87, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    6 /* Public */,
       3,    0,   69,    2, 0x06,    7 /* Public */,
       4,    0,   70,    2, 0x06,    8 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       5,    0,   71,    2, 0x02,    9 /* Public */,
       6,    2,   72,    2, 0x02,   10 /* Public */,
       9,    1,   77,    2, 0x02,   13 /* Public */,
      11,    1,   80,    2, 0x02,   15 /* Public */,
      13,    0,   83,    2, 0x02,   17 /* Public */,
      14,    1,   84,    2, 0x02,   18 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    7,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Double,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   15,

 // properties: name, type, flags
      16, QMetaType::QString, 0x00015103, uint(0), 0,
      17, QMetaType::QVariantList, 0x00015001, uint(1), 0,
      18, QMetaType::Bool, 0x00015001, uint(1), 0,
      19, QMetaType::QString, 0x00015001, uint(1), 0,
      20, QMetaType::QVariantList, 0x00015001, uint(2), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTTweaksAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTTweaksAdapter.offsetsAndSizes,
    qt_meta_data_LTTweaksAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTTweaksAdapter_t,
        // property 'deviceUuid'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'tweaks'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'running'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'message'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'hiddenApps'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTTweaksAdapter, std::true_type>,
        // method 'deviceUuidChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'tweaksChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'hiddenAppsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setTweak'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setColor'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setClock'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'listHiddenApps'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openApp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LTTweaksAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTTweaksAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceUuidChanged(); break;
        case 1: _t->tweaksChanged(); break;
        case 2: _t->hiddenAppsChanged(); break;
        case 3: _t->refresh(); break;
        case 4: _t->setTweak((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 5: _t->setColor((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->setClock((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->listHiddenApps(); break;
        case 8: _t->openApp((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTTweaksAdapter::*)();
            if (_t _q_method = &LTTweaksAdapter::deviceUuidChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTTweaksAdapter::*)();
            if (_t _q_method = &LTTweaksAdapter::tweaksChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LTTweaksAdapter::*)();
            if (_t _q_method = &LTTweaksAdapter::hiddenAppsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTTweaksAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->deviceUuid(); break;
        case 1: *reinterpret_cast< QVariantList*>(_v) = _t->tweaks(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->running(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->message(); break;
        case 4: *reinterpret_cast< QVariantList*>(_v) = _t->hiddenApps(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTTweaksAdapter *>(_o);
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

const QMetaObject *LTTweaksAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTTweaksAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTTweaksAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTTweaksAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
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
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void LTTweaksAdapter::deviceUuidChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTTweaksAdapter::tweaksChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LTTweaksAdapter::hiddenAppsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
