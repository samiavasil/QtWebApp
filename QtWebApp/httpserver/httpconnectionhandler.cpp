/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"


using namespace stefanfrings;

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
    busy=false;

    // Create TCP or SSL socket
    createSocket();

    // execute signals in my own thread
#if defined(HANDLER_THREADING)
    moveToThread(this);
    socket->moveToThread(this);
    readTimer.moveToThread(this);
#endif
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    readTimer.setSingleShot(true);

    qDebug("HttpConnectionHandler (%p): constructed", this);
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
    qDebug("HttpConnectionHandler (%p) type (%d): destroyed", this, m_type);
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
            qDebug("HttpConnectionHandler (%p): SSL is enabled", this);
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
    qDebug("HttpConnectionHandler (%p): thread started", this);
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
    qDebug() << "preSharedKeyAuthenticationRequired required";
}

void HttpConnectionHandler::encrypted()
{
    QSslSocket* sslSocket = (QSslSocket*)socket;
    qDebug() << "SSL ENCRYPTED!!!!!!!!!!!!!!";
    connect(sslSocket, SIGNAL(readyRead()), SLOT(read()));
    connect(sslSocket, SIGNAL(disconnected()), SLOT(disconnected()));
}

void HttpConnectionHandler::sslErrors(const QList<QSslError> &errors)
{
    qDebug() << "SSL ERRORS: "  << errors;
   ((  QSslSocket*)socket)->ignoreSslErrors();
}

void HttpConnectionHandler::handleConnection(tSocketDescriptor socketDescriptor)
{
    qDebug("HttpConnectionHandler (%p): handle new connection", this);
    busy = true;
    Q_ASSERT(socket->isOpen()==false); // if not, then the handler is already busy

    //UGLY workaround - we need to clear writebuffer before reusing this socket
    //https://bugreports.qt-project.org/browse/QTBUG-28914
   socket->connectToHost("",0);
   socket->abort();

#ifndef QT_NO_OPENSSL
    if(sslConfiguration)
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
        qDebug("HttpConnectionHandler (%p): SSL is enabled", this);
    }
    else
    {
        // Connect signals
        connect(socket, SIGNAL(readyRead()), this,SLOT(read()));
        connect(socket, SIGNAL(disconnected()), this,SLOT(disconnected()));
    }
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
            qDebug("HttpConnectionHandler (%p): Starting encryption", this);
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

}


bool HttpConnectionHandler::isBusy()
{
    return busy;
}

void HttpConnectionHandler::setBusy()
{
    this->busy = true;
}


void HttpConnectionHandler::readTimeout()
{
    qDebug("!!!!!!!!!!!HttpConnectionHandler (%p) type(%d): read timeout occured",this,m_type);

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
}


void HttpConnectionHandler::disconnected()
{
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
    m_type = UNDEFINED;
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
                qDebug() << disconnect(pTcpSocket, SIGNAL(readyRead()),this, SLOT(read()));
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



void HttpConnectionHandler::readHttp()
{
    // The loop adds support for HTTP pipelinig
    while (socket->bytesAvailable())
    {
        #ifdef SUPERVERBOSE
            qDebug("HttpConnectionHandler (%p): read input",this);
        #endif

        // Create new HttpRequest object if necessary
        if (!currentRequest)
        {
            currentRequest=new HttpRequest(settings);
        }

        // Collect data for the request object
        while (socket->bytesAvailable() && currentRequest->getStatus()!=HttpRequest::complete && currentRequest->getStatus()!=HttpRequest::abort)
        {
            currentRequest->readFromSocket(socket);
            if (currentRequest->getStatus()==HttpRequest::waitForBody)
            {
                // Restart timer for read timeout, otherwise it would
                // expire during large file uploads.
                int readTimeout=settings->value("readTimeout",10000).toInt();
                readTimer.start(readTimeout);
            }
        }

        // If the request is aborted, return error message and close the connection
        if (currentRequest->getStatus()==HttpRequest::abort)
        {
            socket->write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
            socket->flush();
            socket->disconnectFromHost();
            delete currentRequest;
            currentRequest=0;
            return;
        }

        // If the request is complete, let the request mapper dispatch it
        if (currentRequest->getStatus()==HttpRequest::complete)
        {
            readTimer.stop();
            qDebug("HttpConnectionHandler (%p): received request",this);

            // Copy the Connection:close header to the response
            HttpResponse response(socket);
            bool closeConnection=QString::compare(currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0;
            if (closeConnection)
            {
                response.setHeader("Connection","close");
            }

            // In case of HTTP 1.0 protocol add the Connection:close header.
            // This ensures that the HttpResponse does not activate chunked mode, which is not spported by HTTP 1.0.
            else
            {
                bool http1_0=QString::compare(currentRequest->getVersion(),"HTTP/1.0",Qt::CaseInsensitive)==0;
                if (http1_0)
                {
                    closeConnection=true;
                    response.setHeader("Connection","close");
                }
            }

            // Call the request mapper
            try
            {
                requestHandler->service(*currentRequest, response);
            }
            catch (...)
            {
                qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
            }

            // Finalize sending the response if not already done
            if (!response.hasSentLastPart())
            {
                response.write(QByteArray(),true);
            }

            qDebug("HttpConnectionHandler (%p): finished request",this);

            // Find out whether the connection must be closed
            if (!closeConnection)
            {
                // Maybe the request handler or mapper added a Connection:close header in the meantime
                bool closeResponse=QString::compare(response.getHeaders().value("Connection"),"close",Qt::CaseInsensitive)==0;
                if (closeResponse==true)
                {
                    closeConnection=true;
                }
                else
                {
                    // If we have no Content-Length header and did not use chunked mode, then we have to close the
                    // connection to tell the HTTP client that the end of the response has been reached.
                    bool hasContentLength=response.getHeaders().contains("Content-Length");
                    if (!hasContentLength)
                    {
                        bool hasChunkedMode=QString::compare(response.getHeaders().value("Transfer-Encoding"),"chunked",Qt::CaseInsensitive)==0;
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
            }
            else
            {
                // Start timer for next request
                int readTimeout=settings->value("readTimeout",10000).toInt();
                readTimer.start(readTimeout);
            }
            delete currentRequest;
            currentRequest=0;
        }
    }
}


void HttpConnectionHandler::read()
{
    if (!socket->canReadLine()) {
       return;
    }
    if( websocketHandshake( socket ) )
    {
        m_type = WEBSOCKET;
    }
    else
    {
        m_type = HTTP;
        disconnect(socket, SIGNAL(readyRead()),this, SLOT(read()));
        connect(socket, SIGNAL(readyRead()), this,SLOT(readHttp()));
        readHttp();
    }
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
