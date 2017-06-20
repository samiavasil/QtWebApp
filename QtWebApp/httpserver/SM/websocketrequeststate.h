#ifndef WEBSOCKETREQUESTSTATE_H
#define WEBSOCKETREQUESTSTATE_H
#include"connectionstate.h"


namespace SM {

class WebSocketRequestState:public ConnectionState
{
public:
    explicit     WebSocketRequestState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
};
}

#endif // WEBSOCKETREQUESTSTATE_H
