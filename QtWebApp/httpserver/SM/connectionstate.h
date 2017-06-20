#ifndef HTTPCONNECTIONSTATE_H
#define HTTPCONNECTIONSTATE_H
#include"httpconnectionhandler.h"
#include<QString>

namespace SM {

class ConnectionState
{
public:
    explicit ConnectionState(const QString& name );
    virtual ~ConnectionState();
    virtual void handleConnectionEvent( stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor);
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void disconnectEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void writedDataEvent(stefanfrings::HttpConnectionHandler &conHndl , qint64 bytesWriten);
    virtual void asynchronousWorkerEvent( stefanfrings::HttpConnectionHandler &conHndl );

protected:
    QString m_StateName;
};

}

#endif // HTTPCONNECTIONSTATE_H
