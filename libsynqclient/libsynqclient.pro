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
    src/abstractjobprivate.cpp \
    src/abstractwebdavjob.cpp \
    src/abstractwebdavjobprivate.cpp \
    src/createdirectoryjob.cpp \
    src/createdirectoryjobprivate.cpp \
    src/downloadfilejob.cpp \
    src/downloadfilejobprivate.cpp \
    src/getfileinfojob.cpp \
    src/getfileinfojobprivate.cpp \
    src/libsynqclient.cpp \
    src/listfilesjob.cpp \
    src/listfilesjobprivate.cpp \
    src/uploadfilejob.cpp \
    src/uploadfilejobprivate.cpp \
    src/webdavcreatedirectoryjob.cpp \
    src/webdavcreatedirectoryjobprivate.cpp \
    src/webdavdownloadfilejob.cpp \
    src/webdavdownloadfilejobprivate.cpp \
    src/webdavgetfileinfojob.cpp \
    src/webdavgetfileinfojobprivate.cpp \
    src/webdavlistfilesjob.cpp \
    src/webdavlistfilesjobprivate.cpp \
    src/webdavuploadfilejob.cpp \
    src/webdavuploadfilejobprivate.cpp

HEADERS += \
    inc/AbstractJob \
    inc/AbstractWebDAVJob \
    inc/CreateDirectoryJob \
    inc/DownloadFileJob \
    inc/GetFileInfoJob \
    inc/ListFilesJob \
    inc/UploadFileJob \
    inc/WebDAVCreateDirectoryJob \
    inc/WebDAVDownloadFileJob \
    inc/WebDAVGetFileInfoJob \
    inc/WebDAVListFilesJob \
    inc/WebDAVUploadFileJob \
    inc/abstractjob.h \
    inc/abstractwebdavjob.h \
    inc/createdirectoryjob.h \
    inc/downloadfilejob.h \
    inc/getfileinfojob.h \
    inc/libsynqclient_global.h \
    inc/libsynqclient.h \
    inc/listfilesjob.h \
    inc/uploadfilejob.h \
    inc/webdavcreatedirectoryjob.h \
    inc/webdavdownloadfilejob.h \
    inc/webdavgetfileinfojob.h \
    inc/webdavlistfilesjob.h \
    inc/webdavuploadfilejob.h \
    src/abstractjobprivate.h \
    src/abstractwebdavjobprivate.h \
    inc/SynqClient \
    src/createdirectoryjobprivate.h \
    src/downloadfilejobprivate.h \
    src/getfileinfojobprivate.h \
    src/listfilesjobprivate.h \
    src/uploadfilejobprivate.h \
    src/webdavcreatedirectoryjobprivate.h \
    src/webdavdownloadfilejobprivate.h \
    src/webdavgetfileinfojobprivate.h \
    src/webdavlistfilesjobprivate.h \
    src/webdavuploadfilejobprivate.h

INCLUDEPATH += inc

# Default rules for deployment.
target.path = $$[QT_INSTALL_LIBS]
!isEmpty(target.path): INSTALLS += target

headers.files = $$files(inc/*)
headers.path = $$[QT_INSTALL_HEADERS]/SynqClient

