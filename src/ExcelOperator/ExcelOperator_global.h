#ifndef EXCELOPERATOR_GLOBAL_H
#define EXCELOPERATOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXCELOPERATOR_LIBRARY)
#  define EXCELOPERATOR_EXPORT Q_DECL_EXPORT
#else
#  define EXCELOPERATOR_EXPORT Q_DECL_IMPORT
#endif

#endif // EXCELOPERATOR_GLOBAL_H
