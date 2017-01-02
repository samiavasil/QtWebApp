INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

#!win32:QT += network
#win32:LIBS += -luser32

#HEADERS += $$PWD/qtservice.h $$PWD/qtservice_p.h
#unix:HEADERS += $$PWD/qtunixsocket.h $$PWD/qtunixserversocket.h

#SOURCES += $$PWD/qtservice.cpp
#win32:SOURCES += $$PWD/qtservice_win.cpp
#unix:SOURCES += $$PWD/qtservice_unix.cpp $$PWD/qtunixsocket.cpp $$PWD/qtunixserversocket.cpp

#include(qtwebsockets.pro)
# core-private
QT = core-private network
message(  $$PWD )
PUBLIC_HEADERS += \
    $$PWD/qwebsockets_global.h \
    $$PWD/qwebsocket.h \
    $$PWD/qwebsocketserver.h \
    $$PWD/qwebsocketprotocol.h \
    $$PWD/qwebsocketcorsauthenticator.h \
    $$PWD/qmaskgenerator.h

PRIVATE_HEADERS += \
    $$PWD/qwebsocket_p.h \
    $$PWD/qwebsocketserver_p.h \
    $$PWD/qwebsocketprotocol_p.h \
    $$PWD/qwebsockethandshakerequest_p.h \
    $$PWD/qwebsockethandshakeresponse_p.h \
    $$PWD/qwebsocketdataprocessor_p.h \
    $$PWD/qwebsocketcorsauthenticator_p.h \
    $$PWD/qwebsocketframe_p.h \
    $$PWD/qdefaultmaskgenerator_p.h

SOURCES += \
    $$PWD/qwebsocket.cpp \
    $$PWD/qwebsocket_p.cpp \
    $$PWD/qwebsocketserver.cpp \
    $$PWD/qwebsocketserver_p.cpp \
    $$PWD/qwebsocketprotocol.cpp \
    $$PWD/qwebsockethandshakerequest.cpp \
    $$PWD/qwebsockethandshakeresponse.cpp \
    $$PWD/qwebsocketdataprocessor.cpp \
    $$PWD/qwebsocketcorsauthenticator.cpp \
    $$PWD/qwebsocketframe.cpp \
    $$PWD/qmaskgenerator.cpp \
    $$PWD/qdefaultmaskgenerator_p.cpp

#qtConfig(ssl) {
    SOURCES += $$PWD/qsslserver.cpp
    PRIVATE_HEADERS += $$PWD/qsslserver_p.h
#}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
