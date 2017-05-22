#ifndef HTTPCONNECTIONHANDSHAKESTATE_H
#define HTTPCONNECTIONHANDSHAKESTATE_H
#include"httpconnectionstate.h"


namespace stefanfrings {
class HttpConnectionHandshakeState:public HttpConnectionState
{
public:
    explicit     HttpConnectionHandshakeState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};
}

#endif // HTTPCONNECTIONHANDSHAKE_H
