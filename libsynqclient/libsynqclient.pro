TARGET = synqclient
TEMPLATE = lib
QT -= gui
DEFINES += LIBSYNQCLIENT_LIBRARY
CONFIG += hide_symbols create_prl create_pc

# Default rules for deployment.
target.path = $$[QT_INSTALL_LIBS]
!isEmpty(target.path): INSTALLS += target

headers.files = $$files(inc/*)
headers.path = $$[QT_INSTALL_HEADERS]/SynqClient

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(libsynqclient.pri))

