#ifndef WEBSOCKETREQUESTSTATE_H
#define WEBSOCKETREQUESTSTATE_H
#include"connectionstate.h"


namespace SM {

class WebSocketRequestState:public ConnectionState
{
public:
    explicit     WebSocketRequestState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void websocketbinaryFrameReceivedEvent(stefanfrings::HttpConnectionHandler &conHndl, const QByteArray& data, bool final );
    virtual void websocketTextMessageEvent( stefanfrings::HttpConnectionHandler &conHndl, const QString & data);
};
}

#endif // WEBSOCKETREQUESTSTATE_H
