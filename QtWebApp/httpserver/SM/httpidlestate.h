#ifndef HTTPIDLESTATE_H
#define HTTPIDLESTATE_H
#include"connectionstate.h"

namespace SM {
class HttpConnectionHandler;
class HttpIdleState:public ConnectionState
{
public:
    explicit HttpIdleState(const QString& name ):ConnectionState(name){}
    virtual void handleConnectionEvent( stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor);
};
}

#endif // HTTPIDLESTATE_H
