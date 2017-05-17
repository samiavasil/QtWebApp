/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"
#include "httprequest.h"
#include "httprequesthandler.h"
#include "qwebsocket.h"
#include <QSettings>
#include <QTcpSocket>

namespace stefanfrings{

HttpConnectionHandler::HttpConnectionHandler( QSettings* settings, HttpRequestHandler* requestHandler, QSslConfiguration* sslConfiguration, QObject *parent )
    :
#if defined(HANDLER_THREADING)
      QThread(),
#else
      QObject(parent),
#endif
      m_type(UNDEFINED),m_serverName("Websocket Test Server"),socket(NULL),m_WebSocket(NULL)
{
    Q_ASSERT(settings!=0);
    Q_ASSERT(requestHandler!=0);
    this->settings=settings;
    this->requestHandler=requestHandler;
    this->sslConfiguration=sslConfiguration;
    currentRequest=0;
    currentResponse = 0;
    busy=false;
    m_State = IDLE;

    // Create TCP or SSL socket
    createSocket();
//    connect( this->requestHandler, SIGNAL(AsyncEvent()),this, SLOT( handlerSM()) );
    // execute signals in my own thread
#if defined(HANDLER_THREADING)
    moveToThread(this);
    socket->moveToThread(this);
    readTimer.moveToThread(this);
#endif
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    readTimer.setSingleShot(true);

#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p): constructed", this);
#endif

#if defined(HANDLER_THREADING)
    this->start();
#else

#endif
}


HttpConnectionHandler::~HttpConnectionHandler()
{
#if defined(HANDLER_THREADING)
    quit();
    wait();
#else

#endif
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


void HttpConnectionHandler::run()
{
#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p): thread started", this);
#endif
#if defined(HANDLER_THREADING)
    try
    {
        exec();
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): an uncatched exception occured in the thread",this);
    }
    socket->close();
    delete socket;
    readTimer.stop();
    qDebug("HttpConnectionHandler (%p): thread stopped", this);
#else

#endif
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
//    connect(sslSocket, SIGNAL(readyRead()), SLOT(handlerSM()), Qt::QueuedConnection);
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
#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p): handle new connection", this);
#endif
    busy = true;
    Q_ASSERT(socket->isOpen()==false); // if not, then the handler is already busy

    m_State = CONNECT_HANDSHAKE;
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
//        connect(socket, SIGNAL(readyRead()), this,SLOT(handlerSM()), Qt::QueuedConnection);
        connect(socket, SIGNAL(disconnected()), this,SLOT(disconnected()));
    }
    //DEL ME:: TODO just for test
//    connect(this, SIGNAL(dellMeTestSignal()), this,SLOT(handlerSM()),Qt::QueuedConnection);
#else
   // Connect signals
   connect(socket, SIGNAL(readyRead()), this,SLOT(read()));
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
}


void HttpConnectionHandler::disconnected()
{
#if defined SUPERVERBOSE

#endif
    qDebug("!!!!!!!!!!HttpConnectionHandler (%p) type(%d): disconnected", this,m_type);

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
    m_State = IDLE;
    createSocket();
    busy = false;
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
                qDebug() << disconnect(pTcpSocket, SIGNAL(readyRead()),this, SLOT(setDurty()));
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



HttpConnectionHandler::HttpConnectionState HttpConnectionHandler::readHttpRequest()
{
    // The loop adds support for HTTP pipelinig
   // while (socket->bytesAvailable())
    bool callMe = false;
    HttpConnectionHandler::HttpConnectionState state = m_State;
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
                state = HTTP_GET_REQUEST;
            }
        }

        // If the request is aborted, return error message and close the connection
        if (currentRequest->getStatus()==HttpRequest::abort)
        {
            state = HTTP_ABORT;
        }
        if (currentRequest->getStatus()==HttpRequest::complete)
        {
            readTimer.stop();
            qDebug("HttpConnectionHandler (%p): received request",this);
            state = HTTP_HANDLE_REQUEST;
        }
    }
    return state;

}

HttpConnectionHandler::HttpConnectionState HttpConnectionHandler::handleHttpRequest()
{
    // If the request is complete, let the request mapper dispatch it
    if (currentRequest->getStatus()!=HttpRequest::complete)
    {
        qDebug() << "Wrong currentRequest status aborting !!";
        return HTTP_ABORT;
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
           return HTTP_HANDLE_REQUEST;
       }
       else if( HttpRequestHandler::ERROR == reqRet ){
          //TODO: TBD some error. Maybe return some http error....Or try to handle again and after that return http error
       return HTTP_HANDLE_REQUEST;
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
        return CLOSE_CONNECTION;
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
    return HTTP_GET_REQUEST;
}


HttpConnectionHandler::HttpConnectionState HttpConnectionHandler::httpAbort()
{
    socket->write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
    socket->flush();
    socket->disconnectFromHost();
    delete currentRequest;
    currentRequest=0;
    return CONNECT_HANDSHAKE;
}

void HttpConnectionHandler::setDurty()
{
    m_Dirty = true;
}


bool HttpConnectionHandler::handlerSM()
{
    if(  socket->state()!=QAbstractSocket::ConnectedState )
    {
        return false;
    }
    m_Dirty = false;
    switch( m_State )
    {
        case CONNECT_HANDSHAKE:{
            if ( !socket->canReadLine() ) {
                return false;
            }
            if( websocketHandshake( socket ) )
            {
                m_type  = WEBSOCKET;
                m_State = WEBSOCKET_HANDLING;
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
                m_State = HTTP_GET_REQUEST;
                m_State = readHttpRequest();


            }
             m_Dirty = true;
            break;
        }
        case HTTP_GET_REQUEST:{
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
        case HTTP_HANDLE_REQUEST:{
            m_State = handleHttpRequest();
            break;
        }
        case WEBSOCKET_HANDLING:{
            break;
        }
        case HTTP_ABORT:{
            m_State = httpAbort();
            m_Dirty = true;
            break;
        }
        default :{
            break;
        }
    }
    return m_Dirty;
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
