#include "httpconnectionhandshakestate.h"


namespace SM {

HttpConnectionHandshakeState::HttpConnectionHandshakeState(const QString &name):ConnectionState(name){}

void HttpConnectionHandshakeState::handlingLoopEvent(  stefanfrings::HttpConnectionHandler &conHndl )
{
    if ( !conHndl.socket->canReadLine() )
    {
        return;
    }
    if( conHndl.websocketHandshake( conHndl.socket ) )
    {
        conHndl.m_type  = conHndl.WEBSOCKET;
        conHndl.setState(conHndl.WEBSOCKET_REQUEST_STATE);
    }
    else
    {
        conHndl.m_type  = conHndl.HTTP;
        // Create new HttpRequest object if necessary
        if (conHndl.currentRequest)
        {
            delete conHndl.currentRequest;
            conHndl.currentRequest = 0;
        }
        conHndl.setState(conHndl.HTTP_GET_REQUEST_STATE);
        conHndl.requestExecuteSM();
    }
}


void HttpConnectionHandshakeState::readyReadEvent(  stefanfrings::HttpConnectionHandler &conHndl )
{
    conHndl.m_Dirty = true;
}

}
