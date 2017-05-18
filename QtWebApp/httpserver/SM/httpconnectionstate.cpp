#include "httpconnectionstate.h"
#include<QDebug>

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
    qDebug() << "Warning: " << "Event disconnectEvent was received in not appropriate state '" << m_StateName << "':";
    qDebug() << "Info: HttpConnectionHandler = " << &conHndl;
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
