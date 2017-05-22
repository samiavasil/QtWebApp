#ifndef HTTPIDLESTATE_H
#define HTTPIDLESTATE_H
#include"httpconnectionstate.h"

namespace stefanfrings {
class HttpConnectionHandler;
class HttpIdleState:public HttpConnectionState
{
public:
    explicit HttpIdleState(const QString& name ):HttpConnectionState(name){}
    virtual void handleConnectionEvent( stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor);
};
}

#endif // HTTPIDLESTATE_H
