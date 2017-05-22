/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"
#include "httprequest.h"
#include "qwebsocket.h"
#include <QTcpSocket>
#include <SM/httpconnectionstate.h>
#include <SM/httpidlestate.h>
#include <SM/httpconnectionhandshakestate.h>
#include <SM/httpreadrequeststate.h>
#include <SM/httphandlerequeststate.h>


namespace stefanfrings{

HttpConnectionHandler::HttpConnectionHandler( QSettings* settings, HttpRequestHandler* requestHandler, QSslConfiguration* sslConfiguration, QObject *parent )
    : QObject(parent),
      m_type(UNDEFINED),
      m_serverName("Websocket Test Server"),
      socket(NULL),
      m_WebSocket(NULL),
      m_AllStates({ new HttpIdleState("IDLE"),
                    new HttpConnectionHandshakeState("CONNECT_HANDSHAKE"),
                    new HttpReadRequestState("HTTP_GET_REQUEST"),
                    new HttpHandleRequestState("HTTP_HANDLE_REQUEST"),
                    new HttpConnectionState("WEBSOCKET_HANDLING"),
                    new HttpConnectionState("HTTP_ABORT"),
                    new HttpConnectionState("CLOSE_CONNECTION"),
                    })

{
    Q_ASSERT(settings!=0);
    Q_ASSERT(requestHandler!=0);
    setState( IDLE_STATE );
    this->settings=settings;
    this->requestHandler=requestHandler;
    this->sslConfiguration=sslConfiguration;
    currentRequest=0;
    currentResponse = 0;
    busy=false;


    // Create TCP or SSL socket
    createSocket();
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    readTimer.setSingleShot(true);

#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p): constructed", this);
#endif

}


HttpConnectionHandler::~HttpConnectionHandler()
{
    for( int i = 0; i < STATES_NUM; i++ ) {
        if( m_AllStates[i] )
        {
            delete m_AllStates[i];
        }
    }
#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p) type (%d): destroyed", this, m_type);
#endif
}


void HttpConnectionHandler::createSocket()
{
    // If SSL is supported and configured, then create an instance of QSslSocket
    #ifndef QT_NO_OPENSSL
        if (sslConfiguration)
        {
            QSslSocket* sslSocket = NULL;
            if( socket )
            {
                sslSocket = qobject_cast<QSslSocket*>(socket);
                Q_ASSERT( sslSocket );
            }
            else
            {
                sslSocket = new QSslSocket();
                Q_ASSERT( sslSocket );
            }
            sslSocket->setSslConfiguration(*sslConfiguration);
            socket=sslSocket;
#if defined SUPERVERBOSE
            qDebug("HttpConnectionHandler (%p): SSL is enabled", this);
#endif
            return;
        }
    #endif
    // else create an instance of QTcpSocket
    if( NULL == socket )
    {
        socket=new QTcpSocket();
    }

}


void HttpConnectionHandler::preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator)
{
#if defined SUPERVERBOSE
    qDebug() << "preSharedKeyAuthenticationRequired required";
#endif
}

void HttpConnectionHandler::encrypted()
{
    QSslSocket* sslSocket = (QSslSocket*)socket;
#if defined SUPERVERBOSE
    qDebug() << "SSL ENCRYPTED!!!!!!!!!!!!!!";
#endif
    connect(sslSocket, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
    connect(sslSocket, SIGNAL(bytesWritten(qint64)), SLOT(), Qt::QueuedConnection);
    connect(sslSocket, SIGNAL(disconnected()), SLOT(disconnected()), Qt::QueuedConnection );
}

void HttpConnectionHandler::sslErrors(const QList<QSslError> &errors)
{
#if defined SUPERVERBOSE
    qDebug() << "SSL ERRORS: "  << errors;
#endif
   ((  QSslSocket*)socket)->ignoreSslErrors();
}

void HttpConnectionHandler::handleConnection(tSocketDescriptor socketDescriptor)
{
#if defined(SATES_DEFINED)
    m_CurrentConnectionState->handleConnectionEvent(*this, socketDescriptor);
#else

#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p): handle new connection", this);
#endif
    busy = true;
    Q_ASSERT(socket->isOpen()==false); // if not, then the handler is already busy

    m_State = CONNECT_HANDSHAKE_STATE;
    //UGLY workaround - we need to clear writebuffer before reusing this socket
    //https://bugreports.qt-project.org/browse/QTBUG-28914
   socket->connectToHost("",0);
   socket->abort();

#ifndef QT_NO_OPENSSL
    if( sslConfiguration )
    {
        QSslSocket* sslSocket = NULL;
        if( socket )
        {
            sslSocket = qobject_cast<QSslSocket*>(socket);
            Q_ASSERT( sslSocket );
        }
        else
        {
            sslSocket = new QSslSocket();
            Q_ASSERT( sslSocket );
        }
        sslSocket->setSslConfiguration(*sslConfiguration);
        socket=sslSocket;
        connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)),this, SLOT(sslErrors(QList<QSslError>)) );
        connect(sslSocket, SIGNAL(encrypted()), this, SLOT(encrypted()));
        connect(sslSocket, SIGNAL(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator*)),
        this, SLOT(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator*)));
#if defined SUPERVERBOSE
        qDebug("HttpConnectionHandler (%p): SSL is enabled", this);
#endif
    }
    else
    {
        // Connect signals
        connect(socket, SIGNAL(readyRead()), this,SLOT(readyRead()), Qt::QueuedConnection);
        connect(socket, SIGNAL(disconnected()), this,SLOT(disconnected()));
    }
    //DEL ME:: TODO just for test
//    connect(this, SIGNAL(dellMeTestSignal()), this,SLOT(handlerSM()),Qt::QueuedConnection);
#else
   // Connect signals
   connect(socket, SIGNAL(readyRead()), this,SLOT(readyRead()), Qt::QueuedConnection);
   connect(socket, SIGNAL(disconnected()), this,SLOT(disconnected()));
#endif
    if (!socket->setSocketDescriptor(socketDescriptor))
    {
        qCritical("HttpConnectionHandler (%p): cannot initialize socket: %s", this,qPrintable(socket->errorString()));
        return;
    }

    #ifndef QT_NO_OPENSSL
        // Switch on encryption, if SSL is configured
        if (sslConfiguration)
        {
#if defined SUPERVERBOSE
            qDebug("HttpConnectionHandler (%p): Starting encryption", this);
#endif
            QSslSocket* ssl_socket = (QSslSocket*)socket;
            ssl_socket->ignoreSslErrors();
            ssl_socket->startServerEncryption();
        }
    #endif


    // Start timer for read timeout
    int readTimeout=settings->value("readTimeout",10000).toInt();
    readTimer.start(readTimeout);
    // delete previous request
    delete currentRequest;
    currentRequest=0;
    delete currentResponse;
    currentResponse = 0;
#endif
}

void HttpConnectionHandler::readyRead()
{
    m_CurrentConnectionState->readyReadEvent(*this);
}

void HttpConnectionHandler::bytesWritten(quint64 bytesWriten)
{
    m_CurrentConnectionState->writedDataEvent(*this,bytesWriten);
}


bool HttpConnectionHandler::isBusy()
{
    return busy;
}

void HttpConnectionHandler::setBusy()
{
    this->busy = true;
}

bool HttpConnectionHandler::IsDirty() const
{
    return m_Dirty;
}


void HttpConnectionHandler::readTimeout()
{
#if defined SUPERVERBOSE
    qDebug("!!!!!!!!!!!HttpConnectionHandler (%p) type(%d): read timeout occured",this,m_type);
#endif
    //Commented out because QWebView cannot handle this.
    //socket->write("HTTP/1.1 408 request timeout\r\nConnection: close\r\n\r\n408 request timeout\r\n");
disconnected();

//TODO CHECK THIS??
#if 0
    if( WEBSOCKET == m_type )
    {
         m_WebSocket->close();
    }
    else
    {
        socket->flush();
        socket->disconnectFromHost();
    }
    delete currentRequest;
    currentRequest=0;
    delete currentResponse;
    currentResponse = 0;
#endif
}


void HttpConnectionHandler::disconnected()
{
#if defined SUPERVERBOSE
    qDebug("!!!!!!!!!!HttpConnectionHandler (%p) type(%d): disconnected", this,m_type);
#endif   
#if defined(SATES_DEFINED)
    m_CurrentConnectionState->disconnectEvent(*this);
#else
    readTimer.stop();

    if( WEBSOCKET == m_type )
    {
        //m_WebSocket->close();
        m_WebSocket->deleteLater();
        m_WebSocket = NULL;
    }
    else
    {
        socket->close();
        QObject::disconnect(socket,0,0,0);
    }

    m_type  = UNDEFINED;
    m_State = IDLE_STATE;
    createSocket();
    busy = false;
#endif
}

//TODO: Fix me
bool HttpConnectionHandler::websocketHandshake( QTcpSocket *pTcpSocket )
{
    bool ret = false;
    bool isSecure = false;
    QWebSocket* pWebSocket = NULL;

    if( pTcpSocket )
    {
        QByteArray line = pTcpSocket->peek(4096);

        if( line.startsWith("GET ") && line.contains("Upgrade: websocket") )
        {
            pWebSocket = QWebSocket::upgradeFrom( pTcpSocket , m_serverName, isSecure);
            if (pWebSocket)
            {
                m_WebSocket = pWebSocket;
                //TODO  Q_EMIT q->newConnection();
                qDebug("HttpConnectionHandler (%p) change type to WEBSOCKET", this);
                int readTimeout=settings->value("readTimeout",10000).toInt();
                readTimer.start(readTimeout);
                ret = true;
                qDebug() << disconnect(pTcpSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));
                qDebug() << disconnect(pTcpSocket, SIGNAL(disconnected()),this, SLOT(disconnected()));
                connect(m_WebSocket, SIGNAL(textMessageReceived(QString)), SLOT(websocketTextMessage(QString)));
                connect(m_WebSocket, SIGNAL(binaryFrameReceived(QByteArray,bool)), SLOT(websocketbinaryFrameReceived(QByteArray,bool)));
                connect(m_WebSocket, SIGNAL(disconnected()), SLOT(disconnected()));
            }
            else
            {
                qDebug() << tr("ERROR:  Upgrade to WebSocket failed.");
                pTcpSocket->close();
            }
        }
        else
        {
            qDebug() <<  tr("Not WebSocket Request.");
        }
    }
    return  ret;
}

void HttpConnectionHandler::requestExecuteSM()
{
    emit signalExecuteSM();
}


#if !defined(SATES_DEFINED)
HttpConnectionHandler::HttpConnectionStateEnum HttpConnectionHandler::readHttpRequest()
{
    // The loop adds support for HTTP pipelinig
   // while (socket->bytesAvailable())

    HttpConnectionHandler::HttpConnectionStateEnum state = m_State;
    {
        #ifdef SUPERVERBOSE
            qDebug("HttpConnectionHandler (%p): read input",this);
        #endif



        // Collect data for the request object
        if /*while*/(    currentRequest->getStatus()!=HttpRequest::complete && currentRequest->getStatus()!=HttpRequest::abort)
        {
            m_Dirty = currentRequest->readFromSocket(socket) ;
            if (currentRequest->getStatus()==HttpRequest::waitForBody)
            {
                // Restart timer for read timeout, otherwise it would
                // expire during large file uploads.
                int readTimeout=settings->value("readTimeout",10000).toInt();
                readTimer.start(readTimeout);
                state = HTTP_GET_REQUEST_STATE;
            }
        }

        // If the request is aborted, return error message and close the connection
        if (currentRequest->getStatus()==HttpRequest::abort)
        {
            state = HTTP_ABORT_STATE;
        }
        if (currentRequest->getStatus()==HttpRequest::complete)
        {
            readTimer.stop();
            qDebug("HttpConnectionHandler (%p): received request",this);
            state = HTTP_HANDLE_REQUEST_STATE;
        }
    }
    return state;
}


HttpConnectionHandler::HttpConnectionStateEnum HttpConnectionHandler::handleHttpRequest()
{
    // If the request is complete, let the request mapper dispatch it
    if (currentRequest->getStatus()!=HttpRequest::complete)
    {
        qDebug() << "Wrong currentRequest status aborting !!";
        return HTTP_ABORT_STATE;
    }

    if( 0 == currentResponse )
    {
        // Copy the Connection:close header to the response
        currentResponse = new HttpResponse(socket);
        bool closeConnection=QString::compare(currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0;
        if (closeConnection)
        {
            currentResponse->setHeader("Connection","close");
        }

        // In case of HTTP 1.0 protocol add the Connection:close header.
        // This ensures that the HttpResponse does not activate chunked mode, which is not spported by HTTP 1.0.
        else
        {
            bool http1_0=QString::compare(currentRequest->getVersion(),"HTTP/1.0",Qt::CaseInsensitive)==0;
            if (http1_0)
            {
                closeConnection=true;
                currentResponse->setHeader("Connection","close");
            }
        }
    }

    // Call the request mapper
    try
    {
       HttpRequestHandler::ReqHandle_t reqRet = requestHandler->service(*currentRequest, *currentResponse);
       if( HttpRequestHandler::WAIT == reqRet )
       {
           return HTTP_HANDLE_REQUEST_STATE;
       }
       else if( HttpRequestHandler::ERROR == reqRet ){
          //TODO: TBD some error. Maybe return some http error....Or try to handle again and after that return http error
       return HTTP_HANDLE_REQUEST_STATE;
       }
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
    }

    // Finalize sending the currentResponse if not already done
    if (!currentResponse->hasSentLastPart())
    {
        currentResponse->write(QByteArray(),true);
    }

    qDebug("HttpConnectionHandler (%p): finished request",this);

    //TODO: Fix this duplication from gore
    bool closeConnection=QString::compare(currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0;
    // Find out whether the connection must be closed
    if (!closeConnection)
    {
        // Maybe the request handler or mapper added a Connection:close header in the meantime
        bool closeResponse=QString::compare(currentResponse->getHeaders().value("Connection"),"close",Qt::CaseInsensitive)==0;
        if (closeResponse==true)
        {
            closeConnection=true;
        }
        else
        {
            // If we have no Content-Length header and did not use chunked mode, then we have to close the
            // connection to tell the HTTP client that the end of the response has been reached.
            bool hasContentLength=currentResponse->getHeaders().contains("Content-Length");
            if (!hasContentLength)
            {
                bool hasChunkedMode=QString::compare(currentResponse->getHeaders().value("Transfer-Encoding"),"chunked",Qt::CaseInsensitive)==0;
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
        socket->flush();
        socket->disconnectFromHost();
       // return HTTP_GET_REQUEST;
    }
    else
    {
        // Start timer for next request
        int readTimeout=settings->value("readTimeout",10000).toInt();
        readTimer.start(readTimeout);
    }
    delete currentRequest;
    currentRequest=0;
    delete currentResponse;
    currentResponse = 0;
    return HTTP_GET_REQUEST_STATE;
}
#endif

HttpConnectionHandler::HttpConnectionStateEnum HttpConnectionHandler::httpAbort()
{
    socket->write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
    socket->flush();
    delete currentRequest;
    currentRequest=0;
    disconnected();
    /*
    socket->disconnectFromHost();
    delete currentRequest;
    currentRequest=0;
    return CONNECT_HANDSHAKE_STATE;*/
}

void HttpConnectionHandler::AsynchronousTaskFinished()
{
    m_CurrentConnectionState->asynchronousWorkerEvent(*this);
#if !defined(SATES_DEFINED)
    m_Dirty = true;
#endif
}


bool HttpConnectionHandler::handlerSM()
{
#if defined(SATES_DEFINED)
    m_CurrentConnectionState->handlingLoopEvent(*this);
#else
    if(  socket->state()!=QAbstractSocket::ConnectedState )
    {
        return false;
    }
    m_Dirty = false;
    switch( m_State )
    {
        case CONNECT_HANDSHAKE_STATE:{
            if ( !socket->canReadLine() ) {
                return false;
            }
            if( websocketHandshake( socket ) )
            {
                m_type  = WEBSOCKET;
                m_State = WEBSOCKET_HANDLING_STATE;
            }
            else
            {
                m_type  = HTTP;
                // Create new HttpRequest object if necessary
                if (currentRequest)
                {
                   delete currentRequest;
                   currentRequest = 0;
                }
                currentRequest=new HttpRequest(this,settings);
                m_State = HTTP_GET_REQUEST_STATE;
                m_State = readHttpRequest();


            }
             m_Dirty = true;
            break;
        }
        case HTTP_GET_REQUEST_STATE:{
            if (!currentRequest)
            {
               currentRequest=new HttpRequest(this,settings);
            }
            m_State = readHttpRequest();
//            if(m_State == HTTP_HANDLE_REQUEST ){
//                m_Dirty = true;
//            }
            break;
        }
        case HTTP_HANDLE_REQUEST_STATE:{
            m_State = handleHttpRequest();
            break;
        }
        case WEBSOCKET_HANDLING_STATE:{
            break;
        }
        case HTTP_ABORT_STATE:{
            m_State = httpAbort();
            m_Dirty = true;
            break;
        }
        default :{
            break;
        }
    }
#endif
    return m_Dirty;
}

HttpConnectionHandler::HttpConnectionStateEnum HttpConnectionHandler::State() const
{
    return m_State;
}

void HttpConnectionHandler::setState(const HttpConnectionStateEnum &State)
{
    m_State = State;
    m_CurrentConnectionState = m_AllStates[State];
}



void HttpConnectionHandler::websocketTextMessage( const QString & data)
{
    // Call the request mapper
    try
    {
        if(data=="ping")
        {
            m_WebSocket->sendTextMessage("pong");
        }
        else
        {
         //   m_WebSocket->sendTextMessage(data);
            requestHandler->websocketTextMessage( m_WebSocket, data );
        }
        /*Reload read timeout*/
        int readTimeout=settings->value("readTimeout",10000).toInt();
        readTimer.start(readTimeout);
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
    }
    //  qDebug() << "Websocket: Reload timeout";
}

void HttpConnectionHandler::websocketbinaryFrameReceived(const QByteArray& data, bool final )
{
    qDebug() << "Websocket Bynary Message:";
    // Call the request mapper
    try
    {
        requestHandler->websocketbinaryFrameReceived( m_WebSocket, data, final );
        /*Reload read timeout*/
        int readTimeout=settings->value("readTimeout",10000).toInt();
        readTimer.start(readTimeout);
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
    }
}

}
