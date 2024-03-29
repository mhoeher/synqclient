/*
 * Copyright 2020-2021 Martin Hoeher <martin@rpdev.net>
 *
 * This file is part of SynqClient.
 *
 * SynqClient is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * SynqClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SynqClient.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    /**
     * @brief The remote unexpectedly closed the connection.
     *
     * This error indicates that the server closed a network connection in an unexpected way.
     * This can be due to network issues but also be a sign of potential server overload
     * scenarios.
     */
    ServerClosedConnection,

    /**
     * @brief The remove file or folder has been deleted.
     *
     * This error is used to indicate that the remote resource which the job worked on has been
     * deleted.
     */
    ResourceDeleted,

    /**
     * @brief The remote path provided to a job did not point to a folder.
     *
     * This error is used to indicate that the resource that a job was pointed to on the remote
     * did not refer to a folder. For example, not all implementations of the ListFilesJob
     * might support "listing" a remote file.
     */
    RemoteResourceIsNotAFolder,
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
 * @brief Determines the way remote updates are detected.
 *
 * This enum is used to select the way remote updates are detected during a synchronization.
 */
enum class RemoteChangeDetectionMode : quint32 {
    /**
     * @brief Remote folders report a sync attribute.
     *
     * This mode is used e.g. for WebDAV. The assumption is, that the server reports sync attributes
     * also for folders. Whenever a file in that folder or in a sub-folder changes, the sync
     * attribute of that folder changes. This way, the sync can build the list of remote changes by
     * querying the remote root folder. If the sync attribute of that folder changed, we know that
     * some file inside this folder changed, so the sync proceeds to check the files and
     * sub-folders. This procedure is done recursively, to find all remote changes.
     *
     * Note that this modes also works if no sync attributes are reported for folders - however, in
     * this case the sync will have to scan all folders each time a sync is run. This means that the
     * minimum number of queries grows with the number of folders that need to be synchronized.
     * Hence, if a backend does not provide sync attributes on folders, ideally an other mechanism
     * is used to discover remote changes.
     */
    FoldersWithSyncAttributes = 1,

    /**
     * @brief The remote provides continuous updates recursively for the root folder.
     *
     * This mode is used for e.g. Dropbox. In this case, the sync assumes that the remote root
     * folder can be queried recursively and that a cursor is returned such that the next time, only
     * the changes since the previous sync can be queried from the server.
     */
    RootFolderSyncStream,
};

Q_ENUM_NS(RemoteChangeDetectionMode);

/**
 * @enum SynqClient::JobState
 * @brief The states of the job life cycle.
 */
enum class JobState : quint32 {
    /**
     * @brief The initial state of each job.
     *
     * Newly created jobs are in this state. While in this state, jobs shall be set up (e.g.
     * by setting paths to a file to be uploaded or to a remote folder which shall be listed).
     */
    Ready = 0,

    /**
     * @brief The job is currently running.
     *
     * After a job has been configured and started, it transitions to this state.
     */
    Running,

    /**
     * @brief The job has finished.
     *
     * This is the final job state. Once a job finished processing, it transitions into this state.
     * Once a job is in this state, it shall be deleted.
     */
    Finished
};

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
 * of a WebDAV server we are talking to.
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
     * When using this server type, the following changes apply compared to a generic server:
     *
     * - The URL (set via AbstractWebDAVJob::url()) should point to the NextCloud root folder, e.g.
     *   `nextcloud.example.com/`. Jobs will then automatically derive the path to the
     *    WebDAV backend (which would in this case be
     *    `nextcloud.example.com/remote.php/webdav/`.
     */

    OwnCloud = 2
    /**<
     * @brief Assume we talk to an ownCloud instance.
     *
     * When using this server type, the following changes apply compared to a generic server:
     *
     * - The URL (set via AbstractWebDAVJob::url()) should point to the ownCloud root folder, e.g.
     *   `owncloud.example.com/`. Jobs will then automatically derive the path to the
     *    WebDAV backend (which would in this case be
     *    `owncloud.example.com/remote.php/webdav/`.
     */
};

Q_ENUM_NS(WebDAVServerType);

/**
 * @brief The type of log message made by the DirectorySynchronizer.
 *
 * This enum is used to classify the type of log message emitted by the DirectorySynchronizer class
 * during a sync.
 */
enum class SynchronizerLogEntryType : quint32 {

    /**
     * @brief An informatinal message.
     *
     * These messages are used to carry arbitrary information about the sync procedure.
     */
    Information = 0,

    /**
     * @brief A warning.
     *
     * Messages of this type carry an arbitrary string used to warn about an unusual, but
     * non-fatal issue during the sync.
     */
    Warning,

    /**
     * @brief An error occurred.
     *
     * A message of this type carries an arbitrary error string used to report a fatal error which
     * causes the sync to stop.
     */
    Error,

    /**
     * @brief A local folder is being created.
     *
     * Messages of this type carry the path of a folder which is created locally.
     */
    LocalMkDir,

    /**
     * @brief A remote folder is being created.
     *
     * Messages of this type carry the path of a folder which is created remotely.
     */
    RemoteMkDir,

    /**
     * @brief A file or folder is deleted locally.
     *
     * Messages of this type carry the path of a file or folder which is removed locally.
     */
    LocalDelete,

    /**
     * @brief A file or folder is deleted remotely.
     *
     * Messages of this type carry the path of a file or folder which is removed remotely.
     */
    RemoteDelete,

    /**
     * @brief A file is being downloaded.
     *
     * Such a message carries the path of a file which is downloaded.
     */
    Download,

    /**
     * @brief A file is being uploaded.
     *
     * Such a message carries the path of a file which is uploaded.
     */
    Upload

};

Q_ENUM_NS(SynchronizerLogEntryType);

/**
 * @brief Determines error handling of composite jobs.
 *
 * This enumeration is used to configure the error handling of composite jobs (i.e. jobs that
 * consist of multiple other jobs).
 */
enum class CompositeJobErrorMode : quint32 {
    /**
     * @brief Stop as soon as the first child job encounters an error.
     *
     * This option will cause the composite job to run as soon as the first child job encounters an
     * error.
     */
    StopOnFirstError = 0,

    /**
     * @brief Continue processing all child jobs, no matter if an error occurred.
     *
     * This option will cause the composite job to finish running all child jobs even if some of
     * them already finished with an error. The composite will report the error of the first failing
     * child as its own error.
     */
    RunAllJobs,
};

Q_ENUM_NS(CompositeJobErrorMode);

/**
 * @brief Workarounds required to use a specific WebDAV server.
 *
 * The values in this type are used to identify issues that a particular WebDAV server has and where
 * client code might try to workaround locally.
 *
 * @sa WebDAVJobFactory::testServer()
 */
enum class WebDAVWorkaround : quint32 {
    /**
     * @brief No workarounds are required.
     */
    NoWorkarounds = 0,

    /**
     * @brief The server does not recursively update ETags on changes.
     *
     * Usually, ETags on folders should be updated if either the folder itself, an item within it or
     * one (recursively) in a sub-folder changed. Some servers hwoever only report changes ETags for
     * a folder, if a direct child of the folder changes. When running e.g. a sync, a possible
     * workaround is to always follow sub-folders recursively, even if the server reports the same
     * ETag as on the last sync.
     */
    NoRecursiveFolderETags = 1 << 0,

    /**
     * @brief ETags differ between GET and PROPFIND requests.
     *
     * Usually, a server shall report the same ETag for a file, no matter if it has been downloaded
     * via a GET request or if such information has been requested from the server via a PROPFIND
     * request.
     *
     * Some servers generate different ETags depending on which type of request is used. A possible
     * workaround is that the client first issues a PROPFIND and then locally stores the ETag
     * retrieved in that instead of the one from the GET request.
     */
    InconsistentETagsUsingPROPFINDAndGET = 1 << 1,

    /**
     * @brief Derive proper ETags on GET when using Apache servers.
     *
     * Some Apache servers are configured in a way, such that they report insonsistent ETags
     * depending if GET or PROPFIND is used.
     *
     * On PROPFIND, such servers usually report an ETag of the form Y-ZZZZZ. On GET, the server
     * returns an ETag of the form XXXX-Y-ZZZZ. As a workaround, the client can try to transform the
     * GET ETag to the form returned by a PROPFIND.
     */
    DerivePROPFINDETagsFromGETETagsForApache = 1 << 2,

};

Q_ENUM_NS(WebDAVWorkaround);

/**
 * @brief Workarounds needed to work with a particular WebDAV server.
 *
 * This is a flags object which works with the values defined in the WebDAVWorkaround enum.
 */
typedef QFlags<WebDAVWorkaround> WebDAVWorkarounds;
Q_FLAG_NS(WebDAVWorkarounds);

} // namespace SynqClient

#endif // LIBSYNQCLIENT_H
