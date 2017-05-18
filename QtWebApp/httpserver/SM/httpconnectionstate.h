#ifndef HTTPCONNECTIONSTATE_H
#define HTTPCONNECTIONSTATE_H
#include"httpconnectionhandler.h"
#include<QString>

class HttpConnectionState
{
public:
    explicit HttpConnectionState(const QString& name );
    virtual ~HttpConnectionState();
    virtual void handleConnectionEvent( stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor);
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void disconnectEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void writedDataEvent(stefanfrings::HttpConnectionHandler &conHndl , qint64 bytesWriten);
    virtual void asynchronousWorkerEvent( stefanfrings::HttpConnectionHandler &conHndl );

protected:
    QString m_StateName;
};


class HttpIdleState:public HttpConnectionState
{
public:
    explicit HttpIdleState(const QString& name ):HttpConnectionState(name){}
    virtual void handleConnectionEvent( stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor);
};

class HttpConnectionHandshake:public HttpConnectionState
{
public:
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void disconnectEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};

class HttpGetBody:public HttpConnectionState
{
public:
    virtual void handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void disconnectEvent( stefanfrings::HttpConnectionHandler &conHndl );
    virtual void readyReadEvent( stefanfrings::HttpConnectionHandler &conHndl );
};


#endif // HTTPCONNECTIONSTATE_H
