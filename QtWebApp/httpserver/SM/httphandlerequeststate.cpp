#include "httphandlerequeststate.h"

namespace SM {

HttpHandleRequestState::HttpHandleRequestState(const QString &name):ConnectionState(name){}

void HttpHandleRequestState::handlingLoopEvent(  stefanfrings::HttpConnectionHandler &conHndl )
{
    conHndl.m_Dirty = true;
    // If the request is complete, let the request mapper dispatch it
    if( conHndl.currentRequest->getStatus() !=  stefanfrings::HttpRequest::complete )
    {
        qDebug() << "Wrong currentRequest status aborting !!";
        conHndl.httpAbort();
        return;
    }

    if( 0 == conHndl.currentResponse )
    {
        // Copy the Connection:close header to the response
        conHndl.currentResponse = new  stefanfrings::HttpResponse(conHndl.socket);
        bool closeConnection=QString::compare(conHndl.currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0;
        if( closeConnection )
        {
            conHndl.currentResponse->setHeader("Connection","close");
        }

        // In case of HTTP 1.0 protocol add the Connection:close header.
        // This ensures that the HttpResponse does not activate chunked mode, which is not spported by HTTP 1.0.
        else
        {
            bool http1_0=QString::compare( conHndl.currentRequest->getVersion(),"HTTP/1.0",Qt::CaseInsensitive )==0;
            if (http1_0)
            {
                closeConnection=true;
                conHndl.currentResponse->setHeader("Connection","close");
            }
        }
    }

    // Call the request mapper
    try
    {
         stefanfrings::HttpRequestHandler::ReqHandle_t reqRet = conHndl.requestHandler->service(*conHndl.currentRequest, *conHndl.currentResponse);
        if(  stefanfrings::HttpRequestHandler::WAIT == reqRet )
        {
            return;
        }
        else if(  stefanfrings::HttpRequestHandler::ERROR == reqRet ){
            //TODO: TBD some error. Maybe return some http error....Or try to handle again and after that return http error
            qDebug() << "HttpRequestHandler::ERROR: Something is wrong....TODO: Fix error";
            return;
        }
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
    }

    // Finalize sending the currentResponse if not already done
    if( !conHndl.currentResponse->hasSentLastPart() )
    {
        conHndl.currentResponse->write(QByteArray(),true);
    }

    qDebug("HttpConnectionHandler (%p): finished request",this);

    //TODO: Fix this duplication from gore
    bool closeConnection=QString::compare(conHndl.currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0;
    // Find out whether the connection must be closed
    if (!closeConnection)
    {
        // Maybe the request handler or mapper added a Connection:close header in the meantime
        bool closeResponse=QString::compare(conHndl.currentResponse->getHeaders().value("Connection"),"close",Qt::CaseInsensitive)==0;
        if ( closeResponse == true )
        {
            closeConnection = true;
        }
        else
        {
            // If we have no Content-Length header and did not use chunked mode, then we have to close the
            // connection to tell the HTTP client that the end of the response has been reached.
            bool hasContentLength=conHndl.currentResponse->getHeaders().contains("Content-Length");
            if( !hasContentLength )
            {
                bool hasChunkedMode=QString::compare( conHndl.currentResponse->getHeaders().value("Transfer-Encoding"),"chunked",Qt::CaseInsensitive )==0;
                if (!hasChunkedMode)
                {
                    closeConnection=true;
                }
            }
        }
    }

    // Close the connection or prepare for the next request on the same connection.
    if (closeConnection)
    {
        conHndl.socket->flush();
        conHndl.socket->disconnectFromHost();
    }
    else
    {
        // Start timer for next request
        int readTimeout=conHndl.settings->value("readTimeout",10000).toInt();
        conHndl.readTimer.start(readTimeout);
    }
    delete conHndl.currentRequest;
    conHndl.currentRequest=0;
    delete conHndl.currentResponse;
    conHndl.currentResponse = 0;
    conHndl.setState( conHndl.HTTP_GET_REQUEST_STATE );
}

void HttpHandleRequestState::readyReadEvent(  stefanfrings::HttpConnectionHandler &conHndl)
{
    conHndl.m_Dirty = true;

}


}
