#ifndef HTTPREADREQUESTSTATE_H
#define HTTPREADREQUESTSTATE_H

#include"connectionstate.h"

namespace SM {
class HttpReadRequestState:public ConnectionState
{
public:
    explicit     HttpReadRequestState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};
}

#endif // HTTPDISCONNECT_H
