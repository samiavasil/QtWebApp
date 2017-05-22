#include "httpreadrequeststate.h"

namespace stefanfrings {

HttpReadRequestState::HttpReadRequestState(const QString &name):HttpConnectionState(name){}

void HttpReadRequestState::handlingLoopEvent(HttpConnectionHandler &conHndl)
{
    conHndl.m_Dirty = false;
    if( !conHndl.currentRequest )
    {
        conHndl.currentRequest=new HttpRequest(&conHndl,conHndl.settings);
    }
    // Collect data for the request object
    if /*while*/( conHndl.currentRequest->getStatus()!=HttpRequest::complete && conHndl.currentRequest->getStatus()!=HttpRequest::abort)
    {
        conHndl.currentRequest->readFromSocket(conHndl.socket) ;
        if( conHndl.currentRequest->getStatus()==HttpRequest::waitForBody )
        {
            // Restart timer for read timeout, otherwise it would
            // expire during large file uploads.
            int readTimeout=conHndl.settings->value("readTimeout",10000).toInt();
            conHndl.readTimer.start(readTimeout);
        }
    }

    // If the request is aborted, return error message and close the connection
    if( conHndl.currentRequest->getStatus()==HttpRequest::abort )
    {
        conHndl.httpAbort();
        return;
    }
    if( conHndl.currentRequest->getStatus()==HttpRequest::complete )
    {
        conHndl.readTimer.stop();
        qDebug("HttpConnectionHandler (%p): received request",this);
        conHndl.setState( conHndl.HTTP_HANDLE_REQUEST_STATE );
        return;
    }
}

void HttpReadRequestState::readyReadEvent(HttpConnectionHandler &conHndl)
{
    conHndl.m_Dirty = true;

}

}
