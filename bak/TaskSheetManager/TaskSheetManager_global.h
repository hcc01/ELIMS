
#ifndef TASKSHEETMANAGER_GLOBAL_H
#define TASKSHEETMANAGER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TASKSHEETMANAGER_LIBRARY)
#  define TASKSHEETMANAGER_EXPORT Q_DECL_EXPORT
#else
#  define TASKSHEETMANAGER_EXPORT Q_DECL_IMPORT
#endif

#endif // TASKSHEETMANAGER_GLOBAL_H
