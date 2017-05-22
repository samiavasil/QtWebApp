#ifndef HTTPREADREQUESTSTATE_H
#define HTTPREADREQUESTSTATE_H

#include"httpconnectionstate.h"

namespace stefanfrings {
class HttpReadRequestState:public HttpConnectionState
{
public:
    explicit     HttpReadRequestState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};
}

#endif // HTTPDISCONNECT_H
