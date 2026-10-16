/****************************************************************************
** Meta object code from reading C++ file 'LTAppsAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTAppsAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTAppsAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTAppsAdapter_t {
    uint offsetsAndSizes[44];
    char stringdata0[14];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[8];
    char stringdata5[8];
    char stringdata6[8];
    char stringdata7[18];
    char stringdata8[19];
    char stringdata9[7];
    char stringdata10[9];
    char stringdata11[14];
    char stringdata12[19];
    char stringdata13[12];
    char stringdata14[6];
    char stringdata15[12];
    char stringdata16[6];
    char stringdata17[11];
    char stringdata18[5];
    char stringdata19[11];
    char stringdata20[11];
    char stringdata21[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTAppsAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTAppsAdapter_t qt_meta_stringdata_LTAppsAdapter = {
    {
        QT_MOC_LITERAL(0, 13),  // "LTAppsAdapter"
        QT_MOC_LITERAL(14, 17),  // "deviceUuidChanged"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 11),  // "rowsChanged"
        QT_MOC_LITERAL(45, 7),  // "refresh"
        QT_MOC_LITERAL(53, 7),  // "install"
        QT_MOC_LITERAL(61, 7),  // "ipaPath"
        QT_MOC_LITERAL(69, 17),  // "pickAndInstallApp"
        QT_MOC_LITERAL(87, 18),  // "pickAndImportMedia"
        QT_MOC_LITERAL(106, 6),  // "remove"
        QT_MOC_LITERAL(113, 8),  // "bundleId"
        QT_MOC_LITERAL(122, 13),  // "cancelInstall"
        QT_MOC_LITERAL(136, 18),  // "restartSpringBoard"
        QT_MOC_LITERAL(155, 11),  // "importMedia"
        QT_MOC_LITERAL(167, 5),  // "paths"
        QT_MOC_LITERAL(173, 11),  // "storeSearch"
        QT_MOC_LITERAL(185, 5),  // "query"
        QT_MOC_LITERAL(191, 10),  // "deviceUuid"
        QT_MOC_LITERAL(202, 4),  // "rows"
        QT_MOC_LITERAL(207, 10),  // "bannerText"
        QT_MOC_LITERAL(218, 10),  // "installing"
        QT_MOC_LITERAL(229, 15)   // "canQueueInstall"
    },
    "LTAppsAdapter",
    "deviceUuidChanged",
    "",
    "rowsChanged",
    "refresh",
    "install",
    "ipaPath",
    "pickAndInstallApp",
    "pickAndImportMedia",
    "remove",
    "bundleId",
    "cancelInstall",
    "restartSpringBoard",
    "importMedia",
    "paths",
    "storeSearch",
    "query",
    "deviceUuid",
    "rows",
    "bannerText",
    "installing",
    "canQueueInstall"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTAppsAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       5,  101, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    6 /* Public */,
       3,    0,   81,    2, 0x06,    7 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,   82,    2, 0x02,    8 /* Public */,
       5,    1,   83,    2, 0x02,    9 /* Public */,
       7,    0,   86,    2, 0x02,   11 /* Public */,
       8,    0,   87,    2, 0x02,   12 /* Public */,
       9,    1,   88,    2, 0x02,   13 /* Public */,
      11,    1,   91,    2, 0x02,   15 /* Public */,
      12,    0,   94,    2, 0x02,   17 /* Public */,
      13,    1,   95,    2, 0x02,   18 /* Public */,
      15,    1,   98,    2, 0x02,   20 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList,   14,
    QMetaType::QVariantList, QMetaType::QString,   16,

 // properties: name, type, flags
      17, QMetaType::QString, 0x00015103, uint(0), 0,
      18, QMetaType::QVariantList, 0x00015001, uint(1), 0,
      19, QMetaType::QString, 0x00015001, uint(1), 0,
      20, QMetaType::Bool, 0x00015001, uint(1), 0,
      21, QMetaType::Bool, 0x00015001, uint(1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTAppsAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTAppsAdapter.offsetsAndSizes,
    qt_meta_data_LTAppsAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTAppsAdapter_t,
        // property 'deviceUuid'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'rows'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'bannerText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'installing'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'canQueueInstall'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTAppsAdapter, std::true_type>,
        // method 'deviceUuidChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'rowsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'install'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'pickAndInstallApp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'pickAndImportMedia'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'remove'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'cancelInstall'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'restartSpringBoard'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'importMedia'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>,
        // method 'storeSearch'
        QtPrivate::TypeAndForceComplete<QVariantList, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LTAppsAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTAppsAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceUuidChanged(); break;
        case 1: _t->rowsChanged(); break;
        case 2: _t->refresh(); break;
        case 3: _t->install((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->pickAndInstallApp(); break;
        case 5: _t->pickAndImportMedia(); break;
        case 6: _t->remove((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->cancelInstall((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->restartSpringBoard(); break;
        case 9: _t->importMedia((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1]))); break;
        case 10: { QVariantList _r = _t->storeSearch((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTAppsAdapter::*)();
            if (_t _q_method = &LTAppsAdapter::deviceUuidChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTAppsAdapter::*)();
            if (_t _q_method = &LTAppsAdapter::rowsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTAppsAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->deviceUuid(); break;
        case 1: *reinterpret_cast< QVariantList*>(_v) = _t->rows(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->bannerText(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->installing(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->canQueueInstall(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTAppsAdapter *>(_o);
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

const QMetaObject *LTAppsAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTAppsAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTAppsAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTAppsAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void LTAppsAdapter::deviceUuidChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTAppsAdapter::rowsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
