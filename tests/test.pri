QT += testlib network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_$${TESTNAME}.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../libsynqclient/release/ -lsynqclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../libsynqclient/debug/ -lsynqclient
else:unix: LIBS += -L$$OUT_PWD/../../libsynqclient/ -lsynqclient

INCLUDEPATH += $$PWD/../libsynqclient/inc
DEPENDPATH += $$PWD/../libsynqclient/inc
