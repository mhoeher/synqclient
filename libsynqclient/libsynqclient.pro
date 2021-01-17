TARGET = synqclient
TEMPLATE = lib
QT -= gui
DEFINES += LIBSYNQCLIENT_LIBRARY
CONFIG += hide_symbols create_prl create_pc

synqclient_with_static_libs {
    CONFIG += static
}

# Default rules for deployment.
isEmpty(INSTALL_PREFIX) {
    target.path = $$[QT_INSTALL_LIBS]
} else {
    win32 {
        target.path = $$INSTALL_PREFIX/bin
    } else {
        target.path = $$INSTALL_PREFIX/lib
    }
}
INSTALLS += target

headers.files = $$files(inc/*)
headers.path = $$[QT_INSTALL_HEADERS]/SynqClient
INSTALLS += headers

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(libsynqclient.pri))

