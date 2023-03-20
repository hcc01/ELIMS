#ifndef RMMANAGER_GLOBAL_H
#define RMMANAGER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RMMANAGER_LIBRARY)
#  define RMMANAGER_EXPORT Q_DECL_EXPORT
#else
#  define RMMANAGER_EXPORT Q_DECL_IMPORT
#endif

#endif // RMMANAGER_GLOBAL_H
