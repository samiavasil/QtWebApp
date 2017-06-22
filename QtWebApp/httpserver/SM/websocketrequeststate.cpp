#include "websocketrequeststate.h"
#include<QDebug>

namespace SM {
WebSocketRequestState::WebSocketRequestState(const QString &name):ConnectionState(name){}

void WebSocketRequestState::handlingLoopEvent( stefanfrings::HttpConnectionHandler &conHndl )
{
    qDebug() << "Info:  Websocke handler = " << &conHndl;
    conHndl.m_Dirty = false;
}

void WebSocketRequestState::websocketbinaryFrameReceivedEvent(stefanfrings::HttpConnectionHandler &conHndl, const QByteArray &data, bool final)
{
    qDebug() << "Websocket Bynary Message:";
    conHndl.requestHandler->websocketbinaryFrameReceived( conHndl.m_WebSocket, data, final );
    /*Reload read timeout*/
    int readTimeout = conHndl.settings->value("readTimeout",10000).toInt();
    conHndl.readTimer.start(readTimeout);
}

void WebSocketRequestState::websocketTextMessageEvent(stefanfrings::HttpConnectionHandler &conHndl, const QString &data)
{
    if(data=="ping")
    {
        conHndl.m_WebSocket->sendTextMessage("pong");
    }
    else
    {
     //   m_WebSocket->sendTextMessage(data);
        conHndl.requestHandler->websocketTextMessage( conHndl.m_WebSocket, data );
    }
    /*Reload read timeout*/
    int readTimeout = conHndl.settings->value("readTimeout",10000).toInt();
    conHndl.readTimer.start(readTimeout);
}

}


