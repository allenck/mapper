#ifndef FUNCTIONS_GLOBAL_H
#define FUNCTIONS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FUNCTIONS_LIBRARY)
#  define FUNCTIONSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define FUNCTIONSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FUNCTIONS_GLOBAL_H
