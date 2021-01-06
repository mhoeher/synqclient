#ifndef LIBSYNQCLIENT_H
#define LIBSYNQCLIENT_H

#include <QtGlobal>
#include <QObject>
#include <QSharedDataPointer>

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

    /**
     * @brief The remote folder already exists.
     *
     * This error is used by jobs creating remote folders. It indicates that the remote folder
     * already exists.
     */
    FolderExists,
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

    /**
     * @brief Opening the sync state database failed.
     *
     * This error is reported when the synchronizer was not able to open the sync state database
     * when starting the sync.
     *
     * @sa SyncStateDatabase::open()
     */
    FailedOpeningSyncStateDatabase,

    /**
     * @brief Failed to close the sync state database.
     *
     * This error is reported if at the end of the sync, closing the sync state database failed.
     */
    FailedClosingSyncStateDatabase,

    /**
     * @brief Creating the remote folder has failed.
     *
     * This error indicates that an error occurred while creating the remote folder. Creating
     * the remote folder is done when the synchronizer detects we are doing the very first sync.
     */
    FailedCreatingRemoteFolder,

    /**
     * @brief Creating a local folder has failed.
     */
    FailedCreatingLocalFolder,

    /**
     * @brief Looking up entries from the sync state database failed.
     *
     * This error indicates that looking up entries from the sync state database failed.
     *
     * @sa SyncStateDatabase::findEntries()
     */
    SyncStateDatabaseLookupFailed,

    /**
     * @brief Writing to the sync state datbase failed.
     */
    SyncStateDatabaseWriteFailed,

    /**
     * @brief Deleting entries from the sync state database failed.
     */
    SyncStateDatabaseDeleteFailed,

    /**
     * @brief Listing a remote folder failed.
     *
     * This error indicates that listing the entries in a remote folder failed.
     */
    FailedListingRemoteFolder,

    /**
     * @brief Deleting a local file has failed.
     */
    FailedDeletingLocalFile,

    /**
     * @brief Deleting a local folder has failed.
     */
    FailedDeletingLocalFolder,

    /**
     * @brief The sync got stuck.
     *
     * This error indicates that during the sync no further actions could be started due to
     * unfulfilled dependencies.
     */
    Stuck,

    /**
     * @brief Uploading a file failed.
     */
    UploadFailed,

    /**
     * @brief Downloading a file failed.
     */
    DownloadFailed,

    /**
     * @brief Writing to a local file has failed.
     */
    WritingToLocalFileFailed,

    /**
     * @brief Opening a local file failed.
     */
    OpeningLocalFileFailed,

    /**
     * @brief Deleting a remote resource has failed.
     */
    FailedDeletingRemoteResource,
};

Q_ENUM_NS(SynchronizerError);

/**
 * @brief Fine tune execution of a sync.
 *
 * The values in this enumeration are used to fine tune the behavior of the synchronization.
 */
enum class SynchronizerFlag : quint32 {
    /**
     * @brief Used to indicate "no" flag.
     *
     * This can be used to create an empty flags value. It has no
     * option turned on.
     */
    NoFlags = 0,

    /**
     * @brief Create the remote folder on the first sync.
     *
     * If this option is set, the remote folder will be created upon the first sync if it does not
     * yet exist.
     */
    CreateRemoteFolderOnFirstSync = 0x00000001,

    /**
     * @brief Default flags used for synchronization.
     *
     * This is the default set of flags used for synchronization. It
     * includes the following list of flags:
     *
     * - CreateRemoteFolderOnFirstSync
     */
    DefaultFlags = CreateRemoteFolderOnFirstSync,
};

/**
 * @typedef SynchronizerFlags
 * @brief Flags used to fine tune exec execution of a sync.
 * @sa SynchronizerFlag
 */
typedef QFlags<SynchronizerFlag> SynchronizerFlags;

Q_FLAG_NS(SynchronizerFlags);

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

} // namespace SynqClient

#endif // LIBSYNQCLIENT_H
