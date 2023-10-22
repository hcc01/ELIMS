#ifndef NETKITS_GLOBAL_H
#define NETKITS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NETKITS_LIBRARY)
#  define NETKITS_EXPORT Q_DECL_EXPORT
#else
#  define NETKITS_EXPORT Q_DECL_IMPORT
#endif

#endif // NETKITS_GLOBAL_H
