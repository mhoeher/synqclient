#ifndef LIBSYNQCLIENT_GLOBAL_H
#define LIBSYNQCLIENT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBSYNQCLIENT_LIBRARY)
#  define LIBSYNQCLIENT_EXPORT Q_DECL_EXPORT
#else
#  define LIBSYNQCLIENT_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBSYNQCLIENT_GLOBAL_H
