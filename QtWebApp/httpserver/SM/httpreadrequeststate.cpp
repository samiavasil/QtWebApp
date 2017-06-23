#include "httpreadrequeststate.h"

namespace SM {

HttpReadRequestState::HttpReadRequestState(const QString &name):ConnectionState(name){}

void HttpReadRequestState::handlingLoopEvent(  stefanfrings::HttpConnectionHandler &conHndl)
{

    if( !conHndl.currentRequest )
    {
        conHndl.currentRequest=new  stefanfrings::HttpRequest(&conHndl,conHndl.settings);
    }
    // Collect data for the request object
    if /*while*/( conHndl.currentRequest->getStatus()!= stefanfrings::HttpRequest::complete && conHndl.currentRequest->getStatus() !=  stefanfrings::HttpRequest::abort)
    {
        conHndl.currentRequest->readFromSocket(conHndl.socket) ;
        if( conHndl.currentRequest->getStatus()== stefanfrings::HttpRequest::waitForBody )
        {
            // Restart timer for read timeout, otherwise it would
            // expire during large file uploads.
            int readTimeout=conHndl.settings->value("readTimeout",10000).toInt();
            conHndl.readTimer.start(readTimeout);
        }
    }

    // If the request is aborted, return error message and close the connection
    if( conHndl.currentRequest->getStatus() ==  stefanfrings::HttpRequest::abort )
    {
        conHndl.httpAbort();
        return;
    }

    if( conHndl.currentRequest->getStatus() ==  stefanfrings::HttpRequest::complete )
    {
        conHndl.readTimer.stop();
        qDebug("HttpConnectionHandler (%p): received request",this);
        conHndl.setState( conHndl.HTTP_HANDLE_REQUEST_STATE );
        return;
    }
    if( conHndl.currentRequest->getStatus() ==  stefanfrings::HttpRequest::waitForRequest ){
        conHndl.m_Dirty = false;
    }
    else{
        conHndl.m_Dirty = true;
    }
}

void HttpReadRequestState::readyReadEvent(  stefanfrings::HttpConnectionHandler &conHndl )
{
    conHndl.m_Dirty = true;

}

}
