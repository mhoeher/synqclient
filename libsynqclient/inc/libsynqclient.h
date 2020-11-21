#ifndef LIBSYNQCLIENT_H
#define LIBSYNQCLIENT_H

#include <QtGlobal>
#include <QObject>

#include "libsynqclient_global.h"

/**
 * @file
 * @brief Global type and constant definitions.
 */

namespace SynqClient {

Q_NAMESPACE_EXPORT(LIBSYNQCLIENT_EXPORT);

/**
 * @brief Used to encode the type of error a job had during its execution.
 */
enum class JobError : quint32 {
    NoError = 0, //!< Indicates that no error occurred.
    Stopped, //!< The job has been stopped by the user.
    MissingParameter, //!< Some parameters required to run the job are missing.
    InvalidParameter, //!< Some parameters have values which are invalid.
    InvalidResponse, //!< Received an invalid response during an operation.
    NetworkRequestFailed, //!< A request to a server via the network failed with an error.
    Forbidden, //!< A request has been rejected because the user is not allowed to run it.
    ResourceNotFound, //!< The specified resource was not found on the server.
    ServerContentConflict, //!< The server encountered a content conflict.
    SyncAttributeMismatch, //!< Encountered a lost update during upload.
};

Q_ENUM_NS(JobError);

/**
 * @enum SynqClient::JobState
 * @brief JobError
 */
enum class JobState : quint32 { Ready = 0, Running, Finished };

Q_ENUM_NS(JobState);

/**
 * @brief The type of WebDAV server to talk to.
 *
 * It is used to finetune the behaviour depending on the concrete implementation
 * if a WebDAV server we are talking to.
 */
enum class WebDAVServerType : quint32 {
    Generic = 0,
    /**<
     * @brief Do not assume any special server type.
     *
     * Use this if you have an absolute path to the WebDAV server and want to
     * talk to it "as-is".
     */

    NextCloud = 1,
    /**<
     * @brief Assume we talk to a NextCloud instance.
     *
     * Use this is the root URL used points to a NextCloud installation. In this
     * case, the WebDAV specific enpoint part of the URL is appended automatically
     * to the root URL.
     */

    OwnCloud = 2
    /**<
     * @brief Assume we talk to an ownCloud instance.
     *
     * Use this is the root URL used points to an ownCloud installation. In this
     * case, the WebDAV specific enpoint part of the URL is appended automatically
     * to the root URL.
     */
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
