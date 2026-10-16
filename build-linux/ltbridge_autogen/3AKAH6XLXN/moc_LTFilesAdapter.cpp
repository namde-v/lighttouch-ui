/****************************************************************************
** Meta object code from reading C++ file 'LTFilesAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTFilesAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTFilesAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTFilesAdapter_t {
    uint offsetsAndSizes[52];
    char stringdata0[15];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[15];
    char stringdata5[16];
    char stringdata6[13];
    char stringdata7[7];
    char stringdata8[5];
    char stringdata9[5];
    char stringdata10[9];
    char stringdata11[7];
    char stringdata12[15];
    char stringdata13[11];
    char stringdata14[16];
    char stringdata15[16];
    char stringdata16[17];
    char stringdata17[5];
    char stringdata18[11];
    char stringdata19[16];
    char stringdata20[16];
    char stringdata21[12];
    char stringdata22[8];
    char stringdata23[17];
    char stringdata24[13];
    char stringdata25[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTFilesAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTFilesAdapter_t qt_meta_stringdata_LTFilesAdapter = {
    {
        QT_MOC_LITERAL(0, 14),  // "LTFilesAdapter"
        QT_MOC_LITERAL(15, 17),  // "deviceUuidChanged"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 17),  // "connectionChanged"
        QT_MOC_LITERAL(52, 14),  // "entriesChanged"
        QT_MOC_LITERAL(67, 15),  // "transferChanged"
        QT_MOC_LITERAL(83, 12),  // "refreshState"
        QT_MOC_LITERAL(96, 6),  // "follow"
        QT_MOC_LITERAL(103, 4),  // "list"
        QT_MOC_LITERAL(108, 4),  // "path"
        QT_MOC_LITERAL(113, 8),  // "transfer"
        QT_MOC_LITERAL(122, 6),  // "opJson"
        QT_MOC_LITERAL(129, 14),  // "cancelTransfer"
        QT_MOC_LITERAL(144, 10),  // "transferId"
        QT_MOC_LITERAL(155, 15),  // "pickUploadFiles"
        QT_MOC_LITERAL(171, 15),  // "pickDownloadDir"
        QT_MOC_LITERAL(187, 16),  // "pickDownloadFile"
        QT_MOC_LITERAL(204, 4),  // "name"
        QT_MOC_LITERAL(209, 10),  // "deviceUuid"
        QT_MOC_LITERAL(220, 15),  // "connectionState"
        QT_MOC_LITERAL(236, 15),  // "connectedDevice"
        QT_MOC_LITERAL(252, 11),  // "currentPath"
        QT_MOC_LITERAL(264, 7),  // "entries"
        QT_MOC_LITERAL(272, 16),  // "transferFraction"
        QT_MOC_LITERAL(289, 12),  // "transferFile"
        QT_MOC_LITERAL(302, 11)   // "hasTransfer"
    },
    "LTFilesAdapter",
    "deviceUuidChanged",
    "",
    "connectionChanged",
    "entriesChanged",
    "transferChanged",
    "refreshState",
    "follow",
    "list",
    "path",
    "transfer",
    "opJson",
    "cancelTransfer",
    "transferId",
    "pickUploadFiles",
    "pickDownloadDir",
    "pickDownloadFile",
    "name",
    "deviceUuid",
    "connectionState",
    "connectedDevice",
    "currentPath",
    "entries",
    "transferFraction",
    "transferFile",
    "hasTransfer"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTFilesAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       9,  108, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   86,    2, 0x06,   10 /* Public */,
       3,    0,   87,    2, 0x06,   11 /* Public */,
       4,    0,   88,    2, 0x06,   12 /* Public */,
       5,    0,   89,    2, 0x06,   13 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   90,    2, 0x02,   14 /* Public */,
       7,    1,   91,    2, 0x02,   15 /* Public */,
       8,    1,   94,    2, 0x02,   17 /* Public */,
      10,    1,   97,    2, 0x02,   19 /* Public */,
      12,    1,  100,    2, 0x02,   21 /* Public */,
      14,    0,  103,    2, 0x02,   23 /* Public */,
      15,    0,  104,    2, 0x02,   24 /* Public */,
      16,    1,  105,    2, 0x02,   25 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::ULongLong,   13,
    QMetaType::QStringList,
    QMetaType::QString,
    QMetaType::QString, QMetaType::QString,   17,

 // properties: name, type, flags
      18, QMetaType::QString, 0x00015103, uint(0), 0,
      19, QMetaType::Int, 0x00015001, uint(1), 0,
      20, QMetaType::QString, 0x00015001, uint(1), 0,
      21, QMetaType::QString, 0x00015001, uint(2), 0,
      22, QMetaType::QVariantList, 0x00015001, uint(2), 0,
      23, QMetaType::Double, 0x00015001, uint(3), 0,
      24, QMetaType::QString, 0x00015001, uint(3), 0,
      13, QMetaType::ULongLong, 0x00015001, uint(3), 0,
      25, QMetaType::Bool, 0x00015001, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTFilesAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTFilesAdapter.offsetsAndSizes,
    qt_meta_data_LTFilesAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTFilesAdapter_t,
        // property 'deviceUuid'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'connectionState'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'connectedDevice'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'currentPath'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'entries'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'transferFraction'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'transferFile'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'transferId'
        QtPrivate::TypeAndForceComplete<quint64, std::true_type>,
        // property 'hasTransfer'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTFilesAdapter, std::true_type>,
        // method 'deviceUuidChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'connectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'entriesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'transferChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshState'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'follow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'list'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'transfer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'cancelTransfer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint64, std::false_type>,
        // method 'pickUploadFiles'
        QtPrivate::TypeAndForceComplete<QStringList, std::false_type>,
        // method 'pickDownloadDir'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'pickDownloadFile'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LTFilesAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTFilesAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceUuidChanged(); break;
        case 1: _t->connectionChanged(); break;
        case 2: _t->entriesChanged(); break;
        case 3: _t->transferChanged(); break;
        case 4: _t->refreshState(); break;
        case 5: _t->follow((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->list((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->transfer((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->cancelTransfer((*reinterpret_cast< std::add_pointer_t<quint64>>(_a[1]))); break;
        case 9: { QStringList _r = _t->pickUploadFiles();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 10: { QString _r = _t->pickDownloadDir();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 11: { QString _r = _t->pickDownloadFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTFilesAdapter::*)();
            if (_t _q_method = &LTFilesAdapter::deviceUuidChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTFilesAdapter::*)();
            if (_t _q_method = &LTFilesAdapter::connectionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LTFilesAdapter::*)();
            if (_t _q_method = &LTFilesAdapter::entriesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LTFilesAdapter::*)();
            if (_t _q_method = &LTFilesAdapter::transferChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTFilesAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->deviceUuid(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->connectionState(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->connectedDevice(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->currentPath(); break;
        case 4: *reinterpret_cast< QVariantList*>(_v) = _t->entries(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->transferFraction(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->transferFile(); break;
        case 7: *reinterpret_cast< quint64*>(_v) = _t->transferId(); break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->hasTransfer(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTFilesAdapter *>(_o);
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

const QMetaObject *LTFilesAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTFilesAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTFilesAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTFilesAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void LTFilesAdapter::deviceUuidChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTFilesAdapter::connectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LTFilesAdapter::entriesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LTFilesAdapter::transferChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
