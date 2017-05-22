#ifndef HTTPHANDLEREQUESTSTATE_H
#define HTTPHANDLEREQUESTSTATE_H
#include"httpconnectionstate.h"

namespace stefanfrings {

class HttpHandleRequestState:public HttpConnectionState
{
public:
    explicit     HttpHandleRequestState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};

}
#endif // HTTPGETBODY_H
