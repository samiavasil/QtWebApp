#include "websocketrequeststate.h"
#include<QDebug>

namespace SM {
WebSocketRequestState::WebSocketRequestState(const QString &name):ConnectionState(name){}

void WebSocketRequestState::handlingLoopEvent(stefanfrings::HttpConnectionHandler &conHndl)
{
    qDebug() << "Info:  Websocke handler = " << &conHndl;
    conHndl.m_Dirty = false;
}

}


