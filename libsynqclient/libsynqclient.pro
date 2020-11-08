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
    src/getfileinfojob.cpp \
    src/getfileinfojobprivate.cpp \
    src/libsynqclient.cpp \
    src/uploadfilejob.cpp \
    src/uploadfilejobprivate.cpp \
    src/webdavcreatedirectoryjob.cpp \
    src/webdavcreatedirectoryjobprivate.cpp \
    src/webdavgetfileinfojob.cpp \
    src/webdavgetfileinfojobprivate.cpp \
    src/webdavuploadfilejob.cpp \
    src/webdavuploadfilejobprivate.cpp

HEADERS += \
    inc/AbstractJob \
    inc/AbstractWebDAVJob \
    inc/CreateDirectoryJob \
    inc/GetFileInfoJob \
    inc/UploadFileJob \
    inc/WebDAVCreateDirectoryJob \
    inc/WebDAVGetFileInfoJob \
    inc/WebDAVUploadFileJob \
    inc/abstractjob.h \
    inc/abstractwebdavjob.h \
    inc/createdirectoryjob.h \
    inc/getfileinfojob.h \
    inc/libsynqclient_global.h \
    inc/libsynqclient.h \
    inc/uploadfilejob.h \
    inc/webdavcreatedirectoryjob.h \
    inc/webdavgetfileinfojob.h \
    inc/webdavuploadfilejob.h \
    src/abstractjobprivate.h \
    src/abstractwebdavjobprivate.h \
    inc/SynqClient \
    src/createdirectoryjobprivate.h \
    src/getfileinfojobprivate.h \
    src/uploadfilejobprivate.h \
    src/webdavcreatedirectoryjobprivate.h \
    src/webdavgetfileinfojobprivate.h \
    src/webdavuploadfilejobprivate.h

INCLUDEPATH += inc

# Default rules for deployment.
target.path = $$[QT_INSTALL_LIBS]
!isEmpty(target.path): INSTALLS += target

headers.files = $$files(inc/*)
headers.path = $$[QT_INSTALL_HEADERS]/SynqClient

