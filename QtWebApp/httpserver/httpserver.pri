INCLUDEPATH += $$PWD
#INCLUDEPATH += $$PWD/../qtwebsockets/src/websockets
DEPENDPATH += $$PWD
include($$PWD/../qtwebsockets/src/websockets/qtwebsockets.pri)
QT += network

# Enable very detailed debug messages when compiling the debug version
CONFIG(debug, debug|release) {
#    DEFINES += SUPERVERBOSE
}

HEADERS += $$PWD/httpglobal.h \
           $$PWD/httplistener.h \
           $$PWD/httpconnectionhandler.h \
           $$PWD/httpconnectionhandlerpool.h \
           $$PWD/httprequest.h \
           $$PWD/httpresponse.h \
           $$PWD/httpcookie.h \
           $$PWD/httprequesthandler.h \
           $$PWD/httpsession.h \
           $$PWD/httpsessionstore.h \
           $$PWD/staticfilecontroller.h \
    $$PWD/statemachine.h \
    $$PWD/asynchronoustaskrunner.h \
    $$PWD/SM/httpconnectionstate.h \
    $$PWD/SM/httpidlestate.h \
    $$PWD/SM/httpreadrequeststate.h \
    $$PWD/SM/httphandlerequeststate.h \
    $$PWD/SM/httpconnectionhandshakestate.h

SOURCES += $$PWD/httpglobal.cpp \
           $$PWD/httplistener.cpp \
           $$PWD/httpconnectionhandler.cpp \
           $$PWD/httpconnectionhandlerpool.cpp \
           $$PWD/httprequest.cpp \
           $$PWD/httpresponse.cpp \
           $$PWD/httpcookie.cpp \
           $$PWD/httprequesthandler.cpp \
           $$PWD/httpsession.cpp \
           $$PWD/httpsessionstore.cpp \
           $$PWD/staticfilecontroller.cpp \
    $$PWD/statemachine.cpp \
    $$PWD/asynchronoustaskrunner.cpp \
    $$PWD/SM/httpconnectionstate.cpp \
    $$PWD/SM/httpidlestate.cpp \
    $$PWD/SM/httpreadrequeststate.cpp \
    $$PWD/SM/httphandlerequeststate.cpp \
    $$PWD/SM/httpconnectionhandshakestate.cpp
