/****************************************************************************
** Meta object code from reading C++ file 'qlcinplugin.h'
**
** Created: Fri Apr 11 16:09:09 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qlcinplugin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qlcinplugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_QLCInPlugin[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      33,   13,   12,   12, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QLCInPlugin[] = {
    "QLCInPlugin\0\0input,channel,value\0"
    "valueChanged(t_input,t_input_channel,t_input_value)\0"
};

const QMetaObject QLCInPlugin::staticMetaObject = {
    { &QLCPlugin::staticMetaObject, qt_meta_stringdata_QLCInPlugin,
      qt_meta_data_QLCInPlugin, 0 }
};

const QMetaObject *QLCInPlugin::metaObject() const
{
    return &staticMetaObject;
}

void *QLCInPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QLCInPlugin))
	return static_cast<void*>(const_cast< QLCInPlugin*>(this));
    return QLCPlugin::qt_metacast(_clname);
}

int QLCInPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLCPlugin::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: valueChanged((*reinterpret_cast< t_input(*)>(_a[1])),(*reinterpret_cast< t_input_channel(*)>(_a[2])),(*reinterpret_cast< t_input_value(*)>(_a[3]))); break;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void QLCInPlugin::valueChanged(t_input _t1, t_input_channel _t2, t_input_value _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
