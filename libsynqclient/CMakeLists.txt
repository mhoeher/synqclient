set(SYNQCLIENT_SOURCES
    src/abstractdropboxjob.cpp
    src/abstractdropboxjobprivate.cpp
    src/abstractjob.cpp
    src/abstractjobfactory.cpp
    src/abstractjobfactoryprivate.cpp
    src/abstractjobprivate.cpp
    src/abstractwebdavjob.cpp
    src/abstractwebdavjobprivate.cpp
    src/compositejob.cpp
    src/compositejobprivate.cpp
    src/createdirectoryjob.cpp
    src/createdirectoryjobprivate.cpp
    src/deletejob.cpp
    src/deletejobprivate.cpp
    src/directorysynchronizer.cpp
    src/directorysynchronizerprivate.cpp
    src/downloadfilejob.cpp
    src/downloadfilejobprivate.cpp
    src/dropboxcreatedirectoryjob.cpp
    src/dropboxcreatedirectoryjobprivate.cpp
    src/dropboxdeletejob.cpp
    src/dropboxdeletejobprivate.cpp
    src/dropboxdownloadfilejob.cpp
    src/dropboxdownloadfilejobprivate.cpp
    src/dropboxgetfileinfojob.cpp
    src/dropboxgetfileinfojobprivate.cpp
    src/dropboxjobfactory.cpp
    src/dropboxjobfactoryprivate.cpp
    src/dropboxlistfilesjob.cpp
    src/dropboxlistfilesjobprivate.cpp
    src/dropboxuploadfilejob.cpp
    src/dropboxuploadfilejobprivate.cpp
    src/fileinfo.cpp
    src/fileinfoprivate.cpp
    src/getfileinfojob.cpp
    src/getfileinfojobprivate.cpp
    src/jsonsyncstatedatabase.cpp
    src/jsonsyncstatedatabaseprivate.cpp
    src/libsynqclient.cpp
    src/listfilesjob.cpp
    src/listfilesjobprivate.cpp
    src/nextcloudloginflow.cpp
    src/nextcloudloginflowprivate.cpp
    src/sqlsyncstatedatabase.cpp
    src/sqlsyncstatedatabaseprivate.cpp
    src/syncstatedatabase.cpp
    src/syncstatedatabaseprivate.cpp
    src/syncstateentry.cpp
    src/syncstateentryprivate.cpp
    src/uploadfilejob.cpp
    src/uploadfilejobprivate.cpp
    src/webdavcreatedirectoryjob.cpp
    src/webdavcreatedirectoryjobprivate.cpp
    src/webdavdeletejob.cpp
    src/webdavdeletejobprivate.cpp
    src/webdavdownloadfilejob.cpp
    src/webdavdownloadfilejobprivate.cpp
    src/webdavgetfileinfojob.cpp
    src/webdavgetfileinfojobprivate.cpp
    src/webdavjobfactory.cpp
    src/webdavjobfactoryprivate.cpp
    src/webdavlistfilesjob.cpp
    src/webdavlistfilesjobprivate.cpp
    src/webdavuploadfilejob.cpp
    src/webdavuploadfilejobprivate.cpp
)

set(SYNQCLIENT_HEADERS
    inc/SynqClient/AbstractDropboxJob
    inc/SynqClient/abstractdropboxjob.h
    inc/SynqClient/AbstractJob
    inc/SynqClient/abstractjob.h
    inc/SynqClient/AbstractJobFactory
    inc/SynqClient/abstractjobfactory.h
    inc/SynqClient/AbstractWebDAVJob
    inc/SynqClient/abstractwebdavjob.h
    inc/SynqClient/CompositeJob
    inc/SynqClient/compositejob.h
    inc/SynqClient/CreateDirectoryJob
    inc/SynqClient/createdirectoryjob.h
    inc/SynqClient/DeleteJob
    inc/SynqClient/deletejob.h
    inc/SynqClient/DirectorySynchronizer
    inc/SynqClient/directorysynchronizer.h
    inc/SynqClient/DownloadFileJob
    inc/SynqClient/downloadfilejob.h
    inc/SynqClient/DropboxCreateDirectoryJob
    inc/SynqClient/dropboxcreatedirectoryjob.h
    inc/SynqClient/DropboxDeleteJob
    inc/SynqClient/dropboxdeletejob.h
    inc/SynqClient/DropboxDownloadFileJob
    inc/SynqClient/dropboxdownloadfilejob.h
    inc/SynqClient/DropboxGetFileInfoJob
    inc/SynqClient/dropboxgetfileinfojob.h
    inc/SynqClient/DropboxJobFactory
    inc/SynqClient/dropboxjobfactory.h
    inc/SynqClient/DropboxListFilesJob
    inc/SynqClient/dropboxlistfilesjob.h
    inc/SynqClient/DropboxUploadFileJob
    inc/SynqClient/dropboxuploadfilejob.h
    inc/SynqClient/FileInfo
    inc/SynqClient/fileinfo.h
    inc/SynqClient/GetFileInfoJob
    inc/SynqClient/getfileinfojob.h
    inc/SynqClient/JSONSyncStateDatabase
    inc/SynqClient/jsonsyncstatedatabase.h
    inc/SynqClient/libsynqclient_global.h
    inc/SynqClient/libsynqclient.h
    inc/SynqClient/ListFilesJob
    inc/SynqClient/listfilesjob.h
    inc/SynqClient/NextCloudLoginFlow
    inc/SynqClient/nextcloudloginflow.h
    inc/SynqClient/SQLSyncStateDatabase
    inc/SynqClient/sqlsyncstatedatabase.h
    inc/SynqClient/SyncStateDatabase
    inc/SynqClient/syncstatedatabase.h
    inc/SynqClient/SyncStateEntry
    inc/SynqClient/syncstateentry.h
    inc/SynqClient/SynqClient
    inc/SynqClient/UploadFileJob
    inc/SynqClient/uploadfilejob.h
    inc/SynqClient/WebDAVCreateDirectoryJob
    inc/SynqClient/webdavcreatedirectoryjob.h
    inc/SynqClient/WebDAVDeleteJob
    inc/SynqClient/webdavdeletejob.h
    inc/SynqClient/WebDAVDownloadFileJob
    inc/SynqClient/webdavdownloadfilejob.h
    inc/SynqClient/WebDAVGetFileInfoJob
    inc/SynqClient/webdavgetfileinfojob.h
    inc/SynqClient/WebDAVJobFactory
    inc/SynqClient/webdavjobfactory.h
    inc/SynqClient/WebDAVListFilesJob
    inc/SynqClient/webdavlistfilesjob.h
    inc/SynqClient/WebDAVUploadFileJob
    inc/SynqClient/webdavuploadfilejob.h
    src/abstractdropboxjobprivate.h
    src/abstractjobfactoryprivate.h
    src/abstractjobprivate.h
    src/abstractwebdavjobprivate.h
    src/changetree.h
    src/compositejobprivate.h
    src/createdirectoryjobprivate.h
    src/deletejobprivate.h
    src/directorysynchronizerprivate.h
    src/downloadfilejobprivate.h
    src/dropboxcreatedirectoryjobprivate.h
    src/dropboxdeletejobprivate.h
    src/dropboxdownloadfilejobprivate.h
    src/dropboxgetfileinfojobprivate.h
    src/dropboxjobfactoryprivate.h
    src/dropboxlistfilesjobprivate.h
    src/dropboxuploadfilejobprivate.h
    src/fileinfoprivate.h
    src/getfileinfojobprivate.h
    src/jsonsyncstatedatabaseprivate.h
    src/listfilesjobprivate.h
    src/nextcloudloginflowprivate.h
    src/sqlsyncstatedatabaseprivate.h
    src/syncactions.h
    src/syncstatedatabaseprivate.h
    src/syncstateentryprivate.h
    src/uploadfilejobprivate.h
    src/webdavcreatedirectoryjobprivate.h
    src/webdavdeletejobprivate.h
    src/webdavdownloadfilejobprivate.h
    src/webdavgetfileinfojobprivate.h
    src/webdavjobfactoryprivate.h
    src/webdavlistfilesjobprivate.h
    src/webdavuploadfilejobprivate.h
)

add_library(synqclient-qt${QT_VERSION_MAJOR})
target_sources(
    synqclient-qt${QT_VERSION_MAJOR} 
    PRIVATE 
        ${SYNQCLIENT_SOURCES}
        ${SYNQCLIENT_HEADERS}
)
target_link_libraries(
    synqclient-qt${QT_VERSION_MAJOR} 
    PUBLIC 
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Sql
)
target_include_directories(
    synqclient-qt${QT_VERSION_MAJOR}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/>
        $<INSTALL_INTERFACE:include/SynqClient-qt${QT_VERSION_MAJOR}>
)
set_target_properties(
    synqclient-qt${QT_VERSION_MAJOR}
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        SOVERSION ${SYNQCLIENT_VERSION}
)
target_compile_definitions(
    synqclient-qt${QT_VERSION_MAJOR}
    PRIVATE
        LIBSYNQCLIENT_LIBRARY

    PUBLIC
        # Request export of symbols only if we don't build a static library
        $<IF:$<STREQUAL:$<TARGET_PROPERTY:synqclient-qt${QT_VERSION_MAJOR},TYPE>,STATIC_LIBRARY>,LIBSYNQCLIENT_LIBRARY_STATIC,LIBSYNQCLIENT_LIBRARY_SHARED>
)
target_compile_features(
    synqclient-qt${QT_VERSION_MAJOR}
    PUBLIC
        cxx_std_14
)

install(
    TARGETS
        synqclient-qt${QT_VERSION_MAJOR}
    EXPORT
        SynqClientTargets
    LIBRARY DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    DIRECTORY
        inc/SynqClient/
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/SynqClient-qt${QT_VERSION_MAJOR}/SynqClient
)

# Add an alias target, so that including the project in source form in another
# project, is possible as well without writing additional code in the host
# project:
add_library(SynqClient::synqclient-qt${QT_VERSION_MAJOR} ALIAS synqclient-qt${QT_VERSION_MAJOR})