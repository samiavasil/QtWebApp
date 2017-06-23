#include "connectionstate.h"
#include"httpconnectionhandler.h"
#include<QDebug>

namespace SM {

ConnectionState::ConnectionState(const QString &name):m_StateName(name){

}

ConnectionState::~ConnectionState()
{

}

void ConnectionState::handleConnectionEvent(stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor){
    qDebug() << "Warning: " << "Event handleConnection was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info:  socketDescriptor = " << socketDescriptor << "  HttpConnectionHandler = " << &conHndl;
}

void ConnectionState::handlingLoopEvent(stefanfrings::HttpConnectionHandler &conHndl){
    qDebug() << "Warning: " << "Event handlingLoopEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info:  HttpConnectionHandler = " << &conHndl;
}

void ConnectionState::disconnectEvent(stefanfrings::HttpConnectionHandler &conHndl){
    if( false == conHndl.busy  ){
        return;
    }
#if defined SUPERVERBOSE
    qDebug() << "    disconnectEvent() on state: %s" << m_StateName ;
#endif
    conHndl.readTimer.stop();

    if( conHndl.WEBSOCKET == conHndl.m_type )
    {
        conHndl.m_WebSocket->close();
        conHndl.m_WebSocket->deleteLater();
        conHndl.m_WebSocket = NULL;
        if( conHndl.m_WsHandshakeRequest )
        {
            delete conHndl.m_WsHandshakeRequest;
            conHndl.m_WsHandshakeRequest = NULL;
        }
    }
    else
    {
        conHndl.socket->close();
        QObject::disconnect(conHndl.socket,0,0,0);
    }
    conHndl.m_type  =  conHndl.UNDEFINED;
    conHndl.createSocket();
    conHndl.busy = false;
    conHndl.m_Dirty = false;
    QObject::disconnect( &conHndl,SIGNAL(signalExecuteSM()),&conHndl,SLOT(handlerSM()) );
    conHndl.setState( conHndl.IDLE_STATE );

    //TODO: Check this
    delete conHndl.currentRequest;
    conHndl.currentRequest=0;
    delete conHndl.currentResponse;
    conHndl.currentResponse = 0;

}

void ConnectionState::readyReadEvent(stefanfrings::HttpConnectionHandler &conHndl){
    qDebug() << "Warning: " << "Event readyReadEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info:  HttpConnectionHandler = " << &conHndl;
}

void ConnectionState::writedDataEvent(stefanfrings::HttpConnectionHandler &conHndl, qint64 bytesWriten){
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl << "Event writedDataEvent state '" << m_StateName << "': bytesWriten = " << bytesWriten;
}

void ConnectionState::asynchronousWorkerEvent(stefanfrings::HttpConnectionHandler &conHndl){
    qDebug() << "Warning: " << "Event asynchronousWorkerEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl;
}

void ConnectionState::websocketbinaryFrameReceivedEvent(stefanfrings::HttpConnectionHandler &conHndl, const QByteArray &data, bool final)
{
    qDebug() << "Warning: " << "Event websocketbinaryFrameReceived was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl;
}

void ConnectionState::websocketTextMessageEvent(stefanfrings::HttpConnectionHandler &conHndl, const QString &data)
{
    qDebug() << "Warning: " << "Event websocketTextMessage was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl;
}




}
