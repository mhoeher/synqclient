QT -= gui
QT += network xml

TARGET = synqclient
TEMPLATE = lib
DEFINES += LIBSYNQCLIENT_LIBRARY

CONFIG += c++14 hide_symbols create_prl create_pc

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/abstractjob.cpp \
    src/abstractjobfactory.cpp \
    src/abstractjobfactoryprivate.cpp \
    src/abstractjobprivate.cpp \
    src/abstractwebdavjob.cpp \
    src/abstractwebdavjobprivate.cpp \
    src/createdirectoryjob.cpp \
    src/createdirectoryjobprivate.cpp \
    src/deletejob.cpp \
    src/deletejobprivate.cpp \
    src/downloadfilejob.cpp \
    src/downloadfilejobprivate.cpp \
    src/getfileinfojob.cpp \
    src/getfileinfojobprivate.cpp \
    src/jsonsyncstatedatabase.cpp \
    src/jsonsyncstatedatabaseprivate.cpp \
    src/libsynqclient.cpp \
    src/listfilesjob.cpp \
    src/listfilesjobprivate.cpp \
    src/nextcloudloginflow.cpp \
    src/nextcloudloginflowprivate.cpp \
    src/sqlsyncstatedatabase.cpp \
    src/sqlsyncstatedatabaseprivate.cpp \
    src/syncstatedatabase.cpp \
    src/syncstatedatabaseprivate.cpp \
    src/syncstateentry.cpp \
    src/syncstateentryprivate.cpp \
    src/uploadfilejob.cpp \
    src/uploadfilejobprivate.cpp \
    src/webdavcreatedirectoryjob.cpp \
    src/webdavcreatedirectoryjobprivate.cpp \
    src/webdavdeletejob.cpp \
    src/webdavdeletejobprivate.cpp \
    src/webdavdownloadfilejob.cpp \
    src/webdavdownloadfilejobprivate.cpp \
    src/webdavgetfileinfojob.cpp \
    src/webdavgetfileinfojobprivate.cpp \
    src/webdavjobfactory.cpp \
    src/webdavjobfactoryprivate.cpp \
    src/webdavlistfilesjob.cpp \
    src/webdavlistfilesjobprivate.cpp \
    src/webdavuploadfilejob.cpp \
    src/webdavuploadfilejobprivate.cpp

HEADERS += \
    inc/AbstractJob \
    inc/AbstractJobFactory \
    inc/AbstractWebDAVJob \
    inc/CreateDirectoryJob \
    inc/DeleteJob \
    inc/DownloadFileJob \
    inc/GetFileInfoJob \
    inc/JSONSyncStateDatabase \
    inc/ListFilesJob \
    inc/NextCloudLoginFlow \
    inc/SQLSyncStateDatabase \
    inc/SyncStateDatabase \
    inc/SyncStateEntry \
    inc/UploadFileJob \
    inc/WebDAVCreateDirectoryJob \
    inc/WebDAVDeleteJob \
    inc/WebDAVDownloadFileJob \
    inc/WebDAVGetFileInfoJob \
    inc/WebDAVJobFactory \
    inc/WebDAVListFilesJob \
    inc/WebDAVUploadFileJob \
    inc/abstractjob.h \
    inc/abstractjobfactory.h \
    inc/abstractwebdavjob.h \
    inc/createdirectoryjob.h \
    inc/deletejob.h \
    inc/downloadfilejob.h \
    inc/getfileinfojob.h \
    inc/jsonsyncstatedatabase.h \
    inc/libsynqclient_global.h \
    inc/libsynqclient.h \
    inc/listfilesjob.h \
    inc/nextcloudloginflow.h \
    inc/sqlsyncstatedatabase.h \
    inc/syncstatedatabase.h \
    inc/syncstateentry.h \
    inc/uploadfilejob.h \
    inc/webdavcreatedirectoryjob.h \
    inc/webdavdeletejob.h \
    inc/webdavdownloadfilejob.h \
    inc/webdavgetfileinfojob.h \
    inc/webdavjobfactory.h \
    inc/webdavlistfilesjob.h \
    inc/webdavuploadfilejob.h \
    src/abstractjobfactoryprivate.h \
    src/abstractjobprivate.h \
    src/abstractwebdavjobprivate.h \
    inc/SynqClient \
    src/createdirectoryjobprivate.h \
    src/deletejobprivate.h \
    src/downloadfilejobprivate.h \
    src/getfileinfojobprivate.h \
    src/jsonsyncstatedatabaseprivate.h \
    src/listfilesjobprivate.h \
    src/nextcloudloginflowprivate.h \
    src/sqlsyncstatedatabaseprivate.h \
    src/syncstatedatabaseprivate.h \
    src/syncstateentryprivate.h \
    src/uploadfilejobprivate.h \
    src/webdavcreatedirectoryjobprivate.h \
    src/webdavdeletejobprivate.h \
    src/webdavdownloadfilejobprivate.h \
    src/webdavgetfileinfojobprivate.h \
    src/webdavjobfactoryprivate.h \
    src/webdavlistfilesjobprivate.h \
    src/webdavuploadfilejobprivate.h

INCLUDEPATH += inc

# Default rules for deployment.
target.path = $$[QT_INSTALL_LIBS]
!isEmpty(target.path): INSTALLS += target

headers.files = $$files(inc/*)
headers.path = $$[QT_INSTALL_HEADERS]/SynqClient

