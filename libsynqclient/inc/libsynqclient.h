#ifndef LIBSYNQCLIENT_H
#define LIBSYNQCLIENT_H

#include <QtGlobal>
#include <QObject>

#include "libsynqclient_global.h"

namespace SynqClient {

Q_NAMESPACE_EXPORT(LIBSYNQCLIENT_EXPORT);

enum class LIBSYNQCLIENT_EXPORT JobError : quint32 {
    NoError = 0, //!< Indicates that no error occurred.
    Stopped, //!< The job has been stopped by the user.
    MissingParameter, //!< Some parameters required to run the job are missing.
    InvalidParameter, //!< Some parameters have values which are invalid.
    InvalidResponse, //!< Received an invalid response during an operation.
    NetworkRequestFailed, //!< A request to a server via the network failed with an error.
};

Q_ENUM_NS(JobError);

enum class LIBSYNQCLIENT_EXPORT JobState : quint32 { Ready = 0, Running, Finished };

Q_ENUM_NS(JobState);

enum class LIBSYNQCLIENT_EXPORT WebDAVServerType : quint32 {
    Generic = 0,
    NextCloud = 1,
    OwnCloud = 2
};

Q_ENUM_NS(WebDAVServerType);

class LIBSYNQCLIENT_EXPORT ItemProperty
{
public:
    static const char* Valid;
    static const char* Type;
    static const char* SyncAttribute;
    static const char* Name;
    static const char* URL;
};

class LIBSYNQCLIENT_EXPORT ItemType
{
public:
    static const char* Directory;
    static const char* File;
    static const char* Invalid;
};

} // namespace SynqClient

#endif // LIBSYNQCLIENT_H
