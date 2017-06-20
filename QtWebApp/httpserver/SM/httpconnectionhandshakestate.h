#ifndef HTTPCONNECTIONHANDSHAKESTATE_H
#define HTTPCONNECTIONHANDSHAKESTATE_H
#include"connectionstate.h"


namespace SM {
class HttpConnectionHandshakeState:public ConnectionState
{
public:
    explicit     HttpConnectionHandshakeState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};
}

#endif // HTTPCONNECTIONHANDSHAKE_H
