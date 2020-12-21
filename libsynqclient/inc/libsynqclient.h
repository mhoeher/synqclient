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
 * @brief Used to encode the type of error during synchronization.
 *
 * This enumeration holds the various types of errors that might occur during a synchronization
 * between a local and a remote folder.
 *
 * @sa DirectorySynchronizer
 */
enum class SynchronizerError : quint32 {
    NoError = 0, //!< The sync was successful - no error occurred.
    Stopped, //!< The sync has been stopped programatically.
    MissingParameter, //!< Indicates that some properties required for the sync are missing.
    InvalidParameter, //!< Indicates that some properties have invalid values set.
};

Q_ENUM_NS(SynchronizerError);

/**
 * @brief Used to identify a specific type of jobs.
 */
enum class JobType : quint32 {
    Invalid = 0, //!< Indicates an invalid job.
    CreateDirectory, //!< A job to create a new directory.
    DeleteResource, //!< A job to delete a file or directory.
    DownloadFile, //!< A job to download a file.
    UploadFile, //!< A job to upload a file.
    GetFileInfo, //!< A job to get information about a single file or directory.
    ListFiles //!< A job to get information about entries in a folder.
};

Q_ENUM_NS(JobType);

/**
 * @enum SynqClient::JobState
 * @brief JobError
 */
enum class JobState : quint32 { Ready = 0, Running, Finished };

Q_ENUM_NS(JobState);

/**
 * @brief The states of a synchronizer.
 *
 * This enum encodes the states a synchronizer runs through.
 */
enum class SynchronizerState : quint32 {
    Ready = 0, //!< The synchronizer is ready and can be started.
    Running, //!< The synchronization is currently running.
    Finished, //!< The synchronization has finished.
};

Q_ENUM_NS(SynchronizerState);

/**
 * @brief Determines how to proceed in case a sync conflict is detected.
 *
 * During a sync operation, it might happen that a sync conflict is detected. A sync conflict
 * happens if a file is modified both locally and remotely.
 *
 * This type is used to instruct the DirectorySynchronizer how to proceed in case of such a
 * conflict.
 */
enum class SyncConflictStrategy : quint32 {
    /**
     * @brief Use the version of a file the remote provides.
     *
     * Use this strategy if you want remote changes to get precedence over local ones.
     * Using this strategy, upon a conflict the remote file will be downloaded and local changes be
     * replaced.
     */
    RemoteWins = 0,

    /**
     * @brief Use the local version of a file.
     *
     * If this strategy is used, upon a conflict the local version of a file is used and uploaded to
     * the remote.
     */
    LocalWins,
};

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
