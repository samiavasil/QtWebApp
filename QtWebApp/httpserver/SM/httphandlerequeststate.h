#ifndef HTTPHANDLEREQUESTSTATE_H
#define HTTPHANDLEREQUESTSTATE_H
#include"connectionstate.h"

namespace SM {

class HttpHandleRequestState:public ConnectionState
{
public:
    explicit     HttpHandleRequestState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};

}
#endif // HTTPGETBODY_H
