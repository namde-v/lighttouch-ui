/****************************************************************************
** Meta object code from reading C++ file 'LTLogAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/adapters/LTLogAdapter.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LTLogAdapter.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LTLogAdapter_t {
    uint offsetsAndSizes[30];
    char stringdata0[13];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[21];
    char stringdata4[17];
    char stringdata5[13];
    char stringdata6[15];
    char stringdata7[5];
    char stringdata8[5];
    char stringdata9[6];
    char stringdata10[8];
    char stringdata11[14];
    char stringdata12[10];
    char stringdata13[6];
    char stringdata14[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LTLogAdapter_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LTLogAdapter_t qt_meta_stringdata_LTLogAdapter = {
    {
        QT_MOC_LITERAL(0, 12),  // "LTLogAdapter"
        QT_MOC_LITERAL(13, 14),  // "sourcesChanged"
        QT_MOC_LITERAL(28, 0),  // ""
        QT_MOC_LITERAL(29, 20),  // "currentSourceChanged"
        QT_MOC_LITERAL(50, 16),  // "followingChanged"
        QT_MOC_LITERAL(67, 12),  // "linesChanged"
        QT_MOC_LITERAL(80, 14),  // "refreshSources"
        QT_MOC_LITERAL(95, 4),  // "read"
        QT_MOC_LITERAL(100, 4),  // "from"
        QT_MOC_LITERAL(105, 5),  // "clear"
        QT_MOC_LITERAL(111, 7),  // "sources"
        QT_MOC_LITERAL(119, 13),  // "currentSource"
        QT_MOC_LITERAL(133, 9),  // "following"
        QT_MOC_LITERAL(143, 5),  // "lines"
        QT_MOC_LITERAL(149, 6)   // "offset"
    },
    "LTLogAdapter",
    "sourcesChanged",
    "",
    "currentSourceChanged",
    "followingChanged",
    "linesChanged",
    "refreshSources",
    "read",
    "from",
    "clear",
    "sources",
    "currentSource",
    "following",
    "lines",
    "offset"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LTLogAdapter[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       5,   72, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x06,    6 /* Public */,
       3,    0,   63,    2, 0x06,    7 /* Public */,
       4,    0,   64,    2, 0x06,    8 /* Public */,
       5,    0,   65,    2, 0x06,    9 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   66,    2, 0x02,   10 /* Public */,
       7,    1,   67,    2, 0x02,   11 /* Public */,
       7,    0,   70,    2, 0x22,   13 /* Public | MethodCloned */,
       9,    0,   71,    2, 0x02,   14 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::ULongLong,    8,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      10, QMetaType::QStringList, 0x00015001, uint(0), 0,
      11, QMetaType::QString, 0x00015103, uint(1), 0,
      12, QMetaType::Bool, 0x00015103, uint(2), 0,
      13, QMetaType::QStringList, 0x00015001, uint(3), 0,
      14, QMetaType::ULongLong, 0x00015001, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LTLogAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<LTAdapterBase::staticMetaObject>(),
    qt_meta_stringdata_LTLogAdapter.offsetsAndSizes,
    qt_meta_data_LTLogAdapter,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LTLogAdapter_t,
        // property 'sources'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // property 'currentSource'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'following'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'lines'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // property 'offset'
        QtPrivate::TypeAndForceComplete<quint64, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LTLogAdapter, std::true_type>,
        // method 'sourcesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'currentSourceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'followingChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'linesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshSources'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'read'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint64, std::false_type>,
        // method 'read'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clear'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void LTLogAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LTLogAdapter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sourcesChanged(); break;
        case 1: _t->currentSourceChanged(); break;
        case 2: _t->followingChanged(); break;
        case 3: _t->linesChanged(); break;
        case 4: _t->refreshSources(); break;
        case 5: _t->read((*reinterpret_cast< std::add_pointer_t<quint64>>(_a[1]))); break;
        case 6: _t->read(); break;
        case 7: _t->clear(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LTLogAdapter::*)();
            if (_t _q_method = &LTLogAdapter::sourcesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LTLogAdapter::*)();
            if (_t _q_method = &LTLogAdapter::currentSourceChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LTLogAdapter::*)();
            if (_t _q_method = &LTLogAdapter::followingChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LTLogAdapter::*)();
            if (_t _q_method = &LTLogAdapter::linesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LTLogAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QStringList*>(_v) = _t->sources(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->currentSource(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->following(); break;
        case 3: *reinterpret_cast< QStringList*>(_v) = _t->lines(); break;
        case 4: *reinterpret_cast< quint64*>(_v) = _t->offset(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LTLogAdapter *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setCurrentSource(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setFollowing(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *LTLogAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LTLogAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LTLogAdapter.stringdata0))
        return static_cast<void*>(this);
    return LTAdapterBase::qt_metacast(_clname);
}

int LTLogAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LTAdapterBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void LTLogAdapter::sourcesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LTLogAdapter::currentSourceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LTLogAdapter::followingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LTLogAdapter::linesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
