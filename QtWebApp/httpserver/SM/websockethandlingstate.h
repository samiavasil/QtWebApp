#ifndef WEBSOCKETHANDLINHGSTATE_H
#define WEBSOCKETHANDLINHGSTATE_H
#include"connectionstate.h"


namespace SM {

class WebSocketHandlingState:public ConnectionState
{
public:
    explicit     WebSocketHandlingState(const QString& name );
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void websocketbinaryFrameReceivedEvent(stefanfrings::HttpConnectionHandler &conHndl, const QByteArray& data, bool final );
    virtual void websocketTextMessageEvent( stefanfrings::HttpConnectionHandler &conHndl, const QString & data);
};
}

#endif // WEBSOCKETHANDLINHGSTATE_H
