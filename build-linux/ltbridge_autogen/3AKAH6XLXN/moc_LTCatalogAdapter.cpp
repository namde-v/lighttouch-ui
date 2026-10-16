/****************************************************************************
** Meta object code from reading C++ file 'LTCatalogAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTCatalogAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTCatalogAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTCatalogAdapter_t {
    uint offsetsAndSizes[24];
    char stringdata0[17];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[15];
    char stringdata5[8];
    char stringdata6[12];
    char stringdata7[8];
    char stringdata8[8];
    char stringdata9[11];
    char stringdata10[8];
    char stringdata11[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTCatalogAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTCatalogAdapter_t qt_meta_stringdata_LTCatalogAdapter = {
    {
        QT_MOC_LITERAL(0, 16),  // "LTCatalogAdapter"
        QT_MOC_LITERAL(17, 14),  // "entriesChanged"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 17),  // "filterTextChanged"
        QT_MOC_LITERAL(51, 14),  // "detailsChanged"
        QT_MOC_LITERAL(66, 7),  // "refresh"
        QT_MOC_LITERAL(74, 11),  // "showDetails"
        QT_MOC_LITERAL(86, 7),  // "entryId"
        QT_MOC_LITERAL(94, 7),  // "entries"
        QT_MOC_LITERAL(102, 10),  // "filterText"
        QT_MOC_LITERAL(113, 7),  // "details"
        QT_MOC_LITERAL(121, 14)   // "detailsEntryId"
    },
    "LTCatalogAdapter",
    "entriesChanged",
    "",
    "filterTextChanged",
    "detailsChanged",
    "refresh",
    "showDetails",
    "entryId",
    "entries",
    "filterText",
    "details",
    "detailsEntryId"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTCatalogAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       4,   51, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    5 /* Public */,
       3,    0,   45,    2, 0x06,    6 /* Public */,
       4,    0,   46,    2, 0x06,    7 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       5,    0,   47,    2, 0x02,    8 /* Public */,
       6,    1,   48,    2, 0x02,    9 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,

 // properties: name, type, flags
       8, QMetaType::QVariantList, 0x00015001, uint(0), 0,
       9, QMetaType::QString, 0x00015103, uint(1), 0,
      10, QMetaType::QVariantMap, 0x00015001, uint(2), 0,
      11, QMetaType::QString, 0x00015001, uint(2), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTCatalogAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTCatalogAdapter.offsetsAndSizes,
    qt_meta_data_LTCatalogAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTCatalogAdapter_t,
        // property 'entries'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'filterText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'details'
        QtPrivate::TypeAndForceComplete<QVariantMap, std::true_type>,
        // property 'detailsEntryId'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTCatalogAdapter, std::true_type>,
        // method 'entriesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'filterTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'detailsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showDetails'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LTCatalogAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTCatalogAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->entriesChanged(); break;
        case 1: _t->filterTextChanged(); break;
        case 2: _t->detailsChanged(); break;
        case 3: _t->refresh(); break;
        case 4: _t->showDetails((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTCatalogAdapter::*)();
            if (_t _q_method = &LTCatalogAdapter::entriesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTCatalogAdapter::*)();
            if (_t _q_method = &LTCatalogAdapter::filterTextChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LTCatalogAdapter::*)();
            if (_t _q_method = &LTCatalogAdapter::detailsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTCatalogAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QVariantList*>(_v) = _t->entries(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->filterText(); break;
        case 2: *reinterpret_cast< QVariantMap*>(_v) = _t->details(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->detailsEntryId(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTCatalogAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setFilterText(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *LTCatalogAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTCatalogAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTCatalogAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTCatalogAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void LTCatalogAdapter::entriesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTCatalogAdapter::filterTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LTCatalogAdapter::detailsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
