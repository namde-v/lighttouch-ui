/****************************************************************************
** Meta object code from reading C++ file 'LTDeviceAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTDeviceAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTDeviceAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTDeviceAdapter_t {
    uint offsetsAndSizes[100];
    char stringdata0[16];
    char stringdata1[25];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[17];
    char stringdata5[14];
    char stringdata6[15];
    char stringdata7[18];
    char stringdata8[29];
    char stringdata9[14];
    char stringdata10[6];
    char stringdata11[8];
    char stringdata12[5];
    char stringdata13[5];
    char stringdata14[10];
    char stringdata15[7];
    char stringdata16[6];
    char stringdata17[10];
    char stringdata18[7];
    char stringdata19[14];
    char stringdata20[14];
    char stringdata21[18];
    char stringdata22[8];
    char stringdata23[17];
    char stringdata24[5];
    char stringdata25[4];
    char stringdata26[12];
    char stringdata27[7];
    char stringdata28[6];
    char stringdata29[14];
    char stringdata30[5];
    char stringdata31[12];
    char stringdata32[8];
    char stringdata33[18];
    char stringdata34[8];
    char stringdata35[13];
    char stringdata36[18];
    char stringdata37[18];
    char stringdata38[13];
    char stringdata39[8];
    char stringdata40[9];
    char stringdata41[13];
    char stringdata42[14];
    char stringdata43[13];
    char stringdata44[14];
    char stringdata45[14];
    char stringdata46[16];
    char stringdata47[18];
    char stringdata48[13];
    char stringdata49[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTDeviceAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTDeviceAdapter_t qt_meta_stringdata_LTDeviceAdapter = {
    {
        QT_MOC_LITERAL(0, 15),  // "LTDeviceAdapter"
        QT_MOC_LITERAL(16, 24),  // "currentDeviceUuidChanged"
        QT_MOC_LITERAL(41, 0),  // ""
        QT_MOC_LITERAL(42, 13),  // "statusChanged"
        QT_MOC_LITERAL(56, 16),  // "bootStageChanged"
        QT_MOC_LITERAL(73, 13),  // "noticeChanged"
        QT_MOC_LITERAL(87, 14),  // "networkChanged"
        QT_MOC_LITERAL(102, 17),  // "fingerDotsChanged"
        QT_MOC_LITERAL(120, 28),  // "networkAccessPromptRequested"
        QT_MOC_LITERAL(149, 13),  // "setFingerDots"
        QT_MOC_LITERAL(163, 5),  // "shown"
        QT_MOC_LITERAL(169, 7),  // "refresh"
        QT_MOC_LITERAL(177, 4),  // "boot"
        QT_MOC_LITERAL(182, 4),  // "halt"
        QT_MOC_LITERAL(187, 9),  // "setPaused"
        QT_MOC_LITERAL(197, 6),  // "paused"
        QT_MOC_LITERAL(204, 5),  // "reset"
        QT_MOC_LITERAL(210, 9),  // "setLocked"
        QT_MOC_LITERAL(220, 6),  // "locked"
        QT_MOC_LITERAL(227, 13),  // "resolveNotice"
        QT_MOC_LITERAL(241, 13),  // "dismissNotice"
        QT_MOC_LITERAL(255, 17),  // "setNetworkAllowed"
        QT_MOC_LITERAL(273, 7),  // "allowed"
        QT_MOC_LITERAL(281, 16),  // "validateMenuItem"
        QT_MOC_LITERAL(298, 4),  // "item"
        QT_MOC_LITERAL(303, 3),  // "arg"
        QT_MOC_LITERAL(307, 11),  // "pressButton"
        QT_MOC_LITERAL(319, 6),  // "button"
        QT_MOC_LITERAL(326, 5),  // "shake"
        QT_MOC_LITERAL(332, 13),  // "setMotionPose"
        QT_MOC_LITERAL(346, 4),  // "pose"
        QT_MOC_LITERAL(351, 11),  // "rotateGuest"
        QT_MOC_LITERAL(363, 7),  // "degrees"
        QT_MOC_LITERAL(371, 17),  // "setCompassHeading"
        QT_MOC_LITERAL(389, 7),  // "heading"
        QT_MOC_LITERAL(397, 12),  // "pasteToGuest"
        QT_MOC_LITERAL(410, 17),  // "refreshFingerDots"
        QT_MOC_LITERAL(428, 17),  // "currentDeviceUuid"
        QT_MOC_LITERAL(446, 12),  // "machineState"
        QT_MOC_LITERAL(459, 7),  // "running"
        QT_MOC_LITERAL(467, 8),  // "sleeping"
        QT_MOC_LITERAL(476, 12),  // "acceptsInput"
        QT_MOC_LITERAL(489, 13),  // "bootStageText"
        QT_MOC_LITERAL(503, 12),  // "bootFraction"
        QT_MOC_LITERAL(516, 13),  // "noticeVisible"
        QT_MOC_LITERAL(530, 13),  // "noticeMessage"
        QT_MOC_LITERAL(544, 15),  // "noticeOperation"
        QT_MOC_LITERAL(560, 17),  // "noticeOffersErase"
        QT_MOC_LITERAL(578, 12),  // "networkState"
        QT_MOC_LITERAL(591, 10)   // "fingerDots"
    },
    "LTDeviceAdapter",
    "currentDeviceUuidChanged",
    "",
    "statusChanged",
    "bootStageChanged",
    "noticeChanged",
    "networkChanged",
    "fingerDotsChanged",
    "networkAccessPromptRequested",
    "setFingerDots",
    "shown",
    "refresh",
    "boot",
    "halt",
    "setPaused",
    "paused",
    "reset",
    "setLocked",
    "locked",
    "resolveNotice",
    "dismissNotice",
    "setNetworkAllowed",
    "allowed",
    "validateMenuItem",
    "item",
    "arg",
    "pressButton",
    "button",
    "shake",
    "setMotionPose",
    "pose",
    "rotateGuest",
    "degrees",
    "setCompassHeading",
    "heading",
    "pasteToGuest",
    "refreshFingerDots",
    "currentDeviceUuid",
    "machineState",
    "running",
    "sleeping",
    "acceptsInput",
    "bootStageText",
    "bootFraction",
    "noticeVisible",
    "noticeMessage",
    "noticeOperation",
    "noticeOffersErase",
    "networkState",
    "fingerDots"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTDeviceAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
      14,  218, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  170,    2, 0x06,   15 /* Public */,
       3,    0,  171,    2, 0x06,   16 /* Public */,
       4,    0,  172,    2, 0x06,   17 /* Public */,
       5,    0,  173,    2, 0x06,   18 /* Public */,
       6,    0,  174,    2, 0x06,   19 /* Public */,
       7,    0,  175,    2, 0x06,   20 /* Public */,
       8,    0,  176,    2, 0x06,   21 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       9,    1,  177,    2, 0x02,   22 /* Public */,
      11,    0,  180,    2, 0x02,   24 /* Public */,
      12,    0,  181,    2, 0x02,   25 /* Public */,
      13,    0,  182,    2, 0x02,   26 /* Public */,
      14,    1,  183,    2, 0x02,   27 /* Public */,
      16,    0,  186,    2, 0x02,   29 /* Public */,
      17,    1,  187,    2, 0x02,   30 /* Public */,
      19,    0,  190,    2, 0x02,   32 /* Public */,
      20,    0,  191,    2, 0x02,   33 /* Public */,
      21,    1,  192,    2, 0x02,   34 /* Public */,
      23,    2,  195,    2, 0x02,   36 /* Public */,
      23,    1,  200,    2, 0x22,   39 /* Public | MethodCloned */,
      26,    1,  203,    2, 0x02,   41 /* Public */,
      28,    0,  206,    2, 0x02,   43 /* Public */,
      29,    1,  207,    2, 0x02,   44 /* Public */,
      31,    1,  210,    2, 0x02,   46 /* Public */,
      33,    1,  213,    2, 0x02,   48 /* Public */,
      35,    0,  216,    2, 0x02,   50 /* Public */,
      36,    0,  217,    2, 0x02,   51 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::Bool,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   22,
    QMetaType::QVariantMap, QMetaType::Int, QMetaType::Int,   24,   25,
    QMetaType::QVariantMap, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   30,
    QMetaType::Void, QMetaType::Int,   32,
    QMetaType::Void, QMetaType::Int,   34,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      37, QMetaType::QString, 0x00015103, uint(0), 0,
      38, QMetaType::Int, 0x00015001, uint(1), 0,
      39, QMetaType::Bool, 0x00015001, uint(1), 0,
      40, QMetaType::Bool, 0x00015001, uint(1), 0,
      15, QMetaType::Bool, 0x00015001, uint(1), 0,
      41, QMetaType::Bool, 0x00015001, uint(1), 0,
      42, QMetaType::QString, 0x00015001, uint(2), 0,
      43, QMetaType::Double, 0x00015001, uint(2), 0,
      44, QMetaType::Bool, 0x00015001, uint(3), 0,
      45, QMetaType::QString, 0x00015001, uint(3), 0,
      46, QMetaType::QString, 0x00015001, uint(3), 0,
      47, QMetaType::Bool, 0x00015001, uint(3), 0,
      48, QMetaType::Int, 0x00015001, uint(4), 0,
      49, QMetaType::Bool, 0x00015103, uint(5), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTDeviceAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTDeviceAdapter.offsetsAndSizes,
    qt_meta_data_LTDeviceAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTDeviceAdapter_t,
        // property 'currentDeviceUuid'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'machineState'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'running'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'sleeping'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'paused'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'acceptsInput'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'bootStageText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'bootFraction'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'noticeVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'noticeMessage'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'noticeOperation'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'noticeOffersErase'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'networkState'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'fingerDots'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTDeviceAdapter, std::true_type>,
        // method 'currentDeviceUuidChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'statusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'bootStageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'noticeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'networkChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fingerDotsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'networkAccessPromptRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setFingerDots'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'boot'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'halt'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setPaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'reset'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setLocked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'resolveNotice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dismissNotice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setNetworkAllowed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'validateMenuItem'
        QtPrivate::TypeAndForceComplete<QVariantMap, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'validateMenuItem'
        QtPrivate::TypeAndForceComplete<QVariantMap, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'pressButton'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'shake'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setMotionPose'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'rotateGuest'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setCompassHeading'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'pasteToGuest'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshFingerDots'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void LTDeviceAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTDeviceAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->currentDeviceUuidChanged(); break;
        case 1: _t->statusChanged(); break;
        case 2: _t->bootStageChanged(); break;
        case 3: _t->noticeChanged(); break;
        case 4: _t->networkChanged(); break;
        case 5: _t->fingerDotsChanged(); break;
        case 6: _t->networkAccessPromptRequested(); break;
        case 7: _t->setFingerDots((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->refresh(); break;
        case 9: _t->boot(); break;
        case 10: _t->halt(); break;
        case 11: _t->setPaused((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->reset(); break;
        case 13: _t->setLocked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->resolveNotice(); break;
        case 15: _t->dismissNotice(); break;
        case 16: _t->setNetworkAllowed((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 17: { QVariantMap _r = _t->validateMenuItem((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])));
            if (_a[0]) *reinterpret_cast< QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 18: { QVariantMap _r = _t->validateMenuItem((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 19: _t->pressButton((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->shake(); break;
        case 21: _t->setMotionPose((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->rotateGuest((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->setCompassHeading((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->pasteToGuest(); break;
        case 25: _t->refreshFingerDots(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::currentDeviceUuidChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::statusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::bootStageChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::noticeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::networkChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::fingerDotsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LTDeviceAdapter::*)();
            if (_t _q_method = &LTDeviceAdapter::networkAccessPromptRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTDeviceAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->currentDeviceUuid(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->machineState(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->running(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->sleeping(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->paused(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->acceptsInput(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->bootStageText(); break;
        case 7: *reinterpret_cast< double*>(_v) = _t->bootFraction(); break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->noticeVisible(); break;
        case 9: *reinterpret_cast< QString*>(_v) = _t->noticeMessage(); break;
        case 10: *reinterpret_cast< QString*>(_v) = _t->noticeOperation(); break;
        case 11: *reinterpret_cast< bool*>(_v) = _t->noticeOffersErase(); break;
        case 12: *reinterpret_cast< int*>(_v) = _t->networkState(); break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->fingerDots(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTDeviceAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setCurrentDeviceUuid(*reinterpret_cast< QString*>(_v)); break;
        case 13: _t->setFingerDots(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *LTDeviceAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTDeviceAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTDeviceAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTDeviceAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 26;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void LTDeviceAdapter::currentDeviceUuidChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTDeviceAdapter::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LTDeviceAdapter::bootStageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LTDeviceAdapter::noticeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void LTDeviceAdapter::networkChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void LTDeviceAdapter::fingerDotsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void LTDeviceAdapter::networkAccessPromptRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
