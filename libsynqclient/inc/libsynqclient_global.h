#ifndef LIBSYNQCLIENT_GLOBAL_H
#define LIBSYNQCLIENT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBSYNQCLIENT_LIBRARY_STATIC)
#    define LIBSYNQCLIENT_EXPORT
#else
#    if defined(LIBSYNQCLIENT_LIBRARY)
#        define LIBSYNQCLIENT_EXPORT Q_DECL_EXPORT
#    else
#        define LIBSYNQCLIENT_EXPORT Q_DECL_IMPORT
#    endif
#endif

#endif // LIBSYNQCLIENT_GLOBAL_H
