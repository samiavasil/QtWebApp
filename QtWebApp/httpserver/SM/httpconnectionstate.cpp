#include "httpconnectionstate.h"
#include"httpconnectionhandler.h"
#include<QDebug>

namespace stefanfrings {

HttpConnectionState::HttpConnectionState(const QString &name):m_StateName(name){

}

HttpConnectionState::~HttpConnectionState()
{

}

void HttpConnectionState::handleConnectionEvent(stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor){
    qDebug() << "Warning: " << "Event handleConnection was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info:  socketDescriptor = " << socketDescriptor << "  HttpConnectionHandler = " << &conHndl;
}

void HttpConnectionState::handlingLoopEvent(stefanfrings::HttpConnectionHandler &conHndl){
    qDebug() << "Warning: " << "Event handlingLoopEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info:  HttpConnectionHandler = " << &conHndl;
}

void HttpConnectionState::disconnectEvent(stefanfrings::HttpConnectionHandler &conHndl){
//#if defined SUPERVERBOSE
    qDebug("!!!!!!!!!!HttpConnectionHandler (%p) type(%d): disconnected", this,conHndl.m_type);
//#endif
    conHndl.readTimer.stop();

    if( conHndl.WEBSOCKET == conHndl.m_type )
    {
        conHndl.m_WebSocket->deleteLater();
        conHndl.m_WebSocket = NULL;
    }
    else
    {
        conHndl.socket->close();
        QObject::disconnect(conHndl.socket,0,0,0);
    }
    conHndl.m_type  =  conHndl.UNDEFINED;
    conHndl.createSocket();
    conHndl.busy = false;
    QObject::disconnect( &conHndl,SIGNAL(signalExecuteSM()),&conHndl,SLOT(handlerSM()) );
    conHndl.setState( conHndl.IDLE_STATE );

    //TODO: Check this
    delete conHndl.currentRequest;
    conHndl.currentRequest=0;
    delete conHndl.currentResponse;
    conHndl.currentResponse = 0;
}

void HttpConnectionState::readyReadEvent(stefanfrings::HttpConnectionHandler &conHndl){
    qDebug() << "Warning: " << "Event readyReadEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info:  HttpConnectionHandler = " << &conHndl;
}

void HttpConnectionState::writedDataEvent(stefanfrings::HttpConnectionHandler &conHndl, qint64 bytesWriten){
    qDebug() << "Warning: " << "Event writedDataEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl;
}

void HttpConnectionState::asynchronousWorkerEvent(stefanfrings::HttpConnectionHandler &conHndl){
    qDebug() << "Warning: " << "Event asynchronousWorkerEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl;
}




}
