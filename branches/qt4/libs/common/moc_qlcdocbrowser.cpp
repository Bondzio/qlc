/****************************************************************************
** Meta object code from reading C++ file 'qlcdocbrowser.h'
**
** Created: Fri Apr 11 16:09:06 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qlcdocbrowser.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qlcdocbrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_QLCDocBrowser[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,
      30,   14,   14,   14, 0x0a,
      58,   14,   14,   14, 0x0a,
      69,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QLCDocBrowser[] = {
    "QLCDocBrowser\0\0slotPrevious()\0"
    "slotBackwardAvailable(bool)\0slotNext()\0"
    "slotForwardAvailable(bool)\0"
};

const QMetaObject QLCDocBrowser::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QLCDocBrowser,
      qt_meta_data_QLCDocBrowser, 0 }
};

const QMetaObject *QLCDocBrowser::metaObject() const
{
    return &staticMetaObject;
}

void *QLCDocBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QLCDocBrowser))
	return static_cast<void*>(const_cast< QLCDocBrowser*>(this));
    return QWidget::qt_metacast(_clname);
}

int QLCDocBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slotPrevious(); break;
        case 1: slotBackwardAvailable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: slotNext(); break;
        case 3: slotForwardAvailable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 4;
    }
    return _id;
}
