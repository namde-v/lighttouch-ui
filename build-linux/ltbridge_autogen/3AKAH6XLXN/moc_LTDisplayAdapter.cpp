/****************************************************************************
** Meta object code from reading C++ file 'LTDisplayAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTDisplayAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTDisplayAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTDisplayAdapter_t {
    uint offsetsAndSizes[106];
    char stringdata0[17];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[16];
    char stringdata5[12];
    char stringdata6[15];
    char stringdata7[8];
    char stringdata8[7];
    char stringdata9[8];
    char stringdata10[20];
    char stringdata11[5];
    char stringdata12[15];
    char stringdata13[4];
    char stringdata14[9];
    char stringdata15[8];
    char stringdata16[13];
    char stringdata17[10];
    char stringdata18[6];
    char stringdata19[2];
    char stringdata20[2];
    char stringdata21[8];
    char stringdata22[8];
    char stringdata23[6];
    char stringdata24[5];
    char stringdata25[10];
    char stringdata26[9];
    char stringdata27[5];
    char stringdata28[9];
    char stringdata29[6];
    char stringdata30[6];
    char stringdata31[13];
    char stringdata32[6];
    char stringdata33[7];
    char stringdata34[11];
    char stringdata35[6];
    char stringdata36[7];
    char stringdata37[7];
    char stringdata38[6];
    char stringdata39[6];
    char stringdata40[7];
    char stringdata41[7];
    char stringdata42[15];
    char stringdata43[10];
    char stringdata44[14];
    char stringdata45[11];
    char stringdata46[10];
    char stringdata47[11];
    char stringdata48[12];
    char stringdata49[16];
    char stringdata50[11];
    char stringdata51[13];
    char stringdata52[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTDisplayAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTDisplayAdapter_t qt_meta_stringdata_LTDisplayAdapter = {
    {
        QT_MOC_LITERAL(0, 16),  // "LTDisplayAdapter"
        QT_MOC_LITERAL(17, 17),  // "deviceUuidChanged"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 11),  // "linkChanged"
        QT_MOC_LITERAL(48, 15),  // "rotationChanged"
        QT_MOC_LITERAL(64, 11),  // "zoomChanged"
        QT_MOC_LITERAL(76, 14),  // "frameAvailable"
        QT_MOC_LITERAL(91, 7),  // "refresh"
        QT_MOC_LITERAL(99, 6),  // "zoomIn"
        QT_MOC_LITERAL(106, 7),  // "zoomOut"
        QT_MOC_LITERAL(114, 19),  // "setZoomModeExplicit"
        QT_MOC_LITERAL(134, 4),  // "mode"
        QT_MOC_LITERAL(139, 14),  // "setZoomContext"
        QT_MOC_LITERAL(154, 3),  // "fit"
        QT_MOC_LITERAL(158, 8),  // "physical"
        QT_MOC_LITERAL(167, 7),  // "backing"
        QT_MOC_LITERAL(175, 12),  // "drawsNearest"
        QT_MOC_LITERAL(188, 9),  // "sendTouch"
        QT_MOC_LITERAL(198, 5),  // "phase"
        QT_MOC_LITERAL(204, 1),  // "x"
        QT_MOC_LITERAL(206, 1),  // "y"
        QT_MOC_LITERAL(208, 7),  // "touchId"
        QT_MOC_LITERAL(216, 7),  // "sendKey"
        QT_MOC_LITERAL(224, 5),  // "qtKey"
        QT_MOC_LITERAL(230, 4),  // "down"
        QT_MOC_LITERAL(235, 9),  // "modifiers"
        QT_MOC_LITERAL(245, 8),  // "typeText"
        QT_MOC_LITERAL(254, 4),  // "text"
        QT_MOC_LITERAL(259, 8),  // "sendTilt"
        QT_MOC_LITERAL(268, 5),  // "angle"
        QT_MOC_LITERAL(274, 5),  // "pitch"
        QT_MOC_LITERAL(280, 12),  // "setPanelSize"
        QT_MOC_LITERAL(293, 5),  // "width"
        QT_MOC_LITERAL(299, 6),  // "height"
        QT_MOC_LITERAL(306, 10),  // "resizeDrag"
        QT_MOC_LITERAL(317, 5),  // "board"
        QT_MOC_LITERAL(323, 6),  // "startW"
        QT_MOC_LITERAL(330, 6),  // "startH"
        QT_MOC_LITERAL(337, 5),  // "edgeX"
        QT_MOC_LITERAL(343, 5),  // "edgeY"
        QT_MOC_LITERAL(349, 6),  // "deltaX"
        QT_MOC_LITERAL(356, 6),  // "deltaY"
        QT_MOC_LITERAL(363, 14),  // "pointsPerPixel"
        QT_MOC_LITERAL(378, 9),  // "symmetric"
        QT_MOC_LITERAL(388, 13),  // "quarterTurned"
        QT_MOC_LITERAL(402, 10),  // "deviceUuid"
        QT_MOC_LITERAL(413, 9),  // "linkReady"
        QT_MOC_LITERAL(423, 10),  // "frameWidth"
        QT_MOC_LITERAL(434, 11),  // "frameHeight"
        QT_MOC_LITERAL(446, 15),  // "rotationDegrees"
        QT_MOC_LITERAL(462, 10),  // "zoomPoints"
        QT_MOC_LITERAL(473, 12),  // "zoomModeName"
        QT_MOC_LITERAL(486, 8)   // "zoomMode"
    },
    "LTDisplayAdapter",
    "deviceUuidChanged",
    "",
    "linkChanged",
    "rotationChanged",
    "zoomChanged",
    "frameAvailable",
    "refresh",
    "zoomIn",
    "zoomOut",
    "setZoomModeExplicit",
    "mode",
    "setZoomContext",
    "fit",
    "physical",
    "backing",
    "drawsNearest",
    "sendTouch",
    "phase",
    "x",
    "y",
    "touchId",
    "sendKey",
    "qtKey",
    "down",
    "modifiers",
    "typeText",
    "text",
    "sendTilt",
    "angle",
    "pitch",
    "setPanelSize",
    "width",
    "height",
    "resizeDrag",
    "board",
    "startW",
    "startH",
    "edgeX",
    "edgeY",
    "deltaX",
    "deltaY",
    "pointsPerPixel",
    "symmetric",
    "quarterTurned",
    "deviceUuid",
    "linkReady",
    "frameWidth",
    "frameHeight",
    "rotationDegrees",
    "zoomPoints",
    "zoomModeName",
    "zoomMode"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTDisplayAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       8,  187, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  116,    2, 0x06,    9 /* Public */,
       3,    0,  117,    2, 0x06,   10 /* Public */,
       4,    0,  118,    2, 0x06,   11 /* Public */,
       5,    0,  119,    2, 0x06,   12 /* Public */,
       6,    0,  120,    2, 0x06,   13 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,  121,    2, 0x02,   14 /* Public */,
       8,    0,  122,    2, 0x02,   15 /* Public */,
       9,    0,  123,    2, 0x02,   16 /* Public */,
      10,    1,  124,    2, 0x02,   17 /* Public */,
      12,    3,  127,    2, 0x02,   19 /* Public */,
      16,    1,  134,    2, 0x02,   23 /* Public */,
      17,    4,  137,    2, 0x02,   25 /* Public */,
      22,    3,  146,    2, 0x02,   30 /* Public */,
      26,    1,  153,    2, 0x02,   34 /* Public */,
      28,    2,  156,    2, 0x02,   36 /* Public */,
      31,    2,  161,    2, 0x02,   39 /* Public */,
      34,   10,  166,    2, 0x02,   42 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double,   13,   14,   15,
    QMetaType::Bool, QMetaType::Double,   15,
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Double, QMetaType::Int,   18,   19,   20,   21,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   23,   24,   25,
    QMetaType::Void, QMetaType::QString,   27,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   29,   30,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   32,   33,
    QMetaType::QString, QMetaType::QString, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool, QMetaType::Bool,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,

 // properties: name, type, flags
      45, QMetaType::QString, 0x00015103, uint(0), 0,
      46, QMetaType::Bool, 0x00015001, uint(1), 0,
      47, QMetaType::Int, 0x00015001, uint(1), 0,
      48, QMetaType::Int, 0x00015001, uint(1), 0,
      49, QMetaType::Int, 0x00015001, uint(2), 0,
      50, QMetaType::Double, 0x00015001, uint(3), 0,
      51, QMetaType::QString, 0x00015001, uint(3), 0,
      52, QMetaType::QString, 0x00015103, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTDisplayAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTDisplayAdapter.offsetsAndSizes,
    qt_meta_data_LTDisplayAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTDisplayAdapter_t,
        // property 'deviceUuid'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'linkReady'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'frameWidth'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'frameHeight'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'rotationDegrees'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'zoomPoints'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'zoomModeName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'zoomMode'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTDisplayAdapter, std::true_type>,
        // method 'deviceUuidChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'linkChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'rotationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'frameAvailable'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomIn'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomOut'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setZoomModeExplicit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setZoomContext'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'drawsNearest'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'sendTouch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'sendKey'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'typeText'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'sendTilt'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'setPanelSize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'resizeDrag'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void LTDisplayAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTDisplayAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceUuidChanged(); break;
        case 1: _t->linkChanged(); break;
        case 2: _t->rotationChanged(); break;
        case 3: _t->zoomChanged(); break;
        case 4: _t->frameAvailable(); break;
        case 5: _t->refresh(); break;
        case 6: _t->zoomIn(); break;
        case 7: _t->zoomOut(); break;
        case 8: _t->setZoomModeExplicit((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->setZoomContext((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 10: { bool _r = _t->drawsNearest((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->sendTouch((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 12: _t->sendKey((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 13: _t->typeText((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->sendTilt((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 15: _t->setPanelSize((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 16: { QString _r = _t->resizeDrag((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[8])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[9])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[10])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTDisplayAdapter::*)();
            if (_t _q_method = &LTDisplayAdapter::deviceUuidChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTDisplayAdapter::*)();
            if (_t _q_method = &LTDisplayAdapter::linkChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LTDisplayAdapter::*)();
            if (_t _q_method = &LTDisplayAdapter::rotationChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LTDisplayAdapter::*)();
            if (_t _q_method = &LTDisplayAdapter::zoomChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LTDisplayAdapter::*)();
            if (_t _q_method = &LTDisplayAdapter::frameAvailable; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTDisplayAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->deviceUuid(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->linkReady(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->frameWidth(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->frameHeight(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->rotationDegrees(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->zoomPoints(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->zoomModeName(); break;
        case 7: *reinterpret_cast< QString*>(_v) = _t->zoomMode(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTDisplayAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setDeviceUuid(*reinterpret_cast< QString*>(_v)); break;
        case 7: _t->setZoomMode(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *LTDisplayAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTDisplayAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTDisplayAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTDisplayAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void LTDisplayAdapter::deviceUuidChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTDisplayAdapter::linkChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LTDisplayAdapter::rotationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LTDisplayAdapter::zoomChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void LTDisplayAdapter::frameAvailable()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
