/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"
#include "httprequest.h"
#include "qwebsocket.h"
#include <QTcpSocket>
#include <SM/connectionstate.h>
#include <SM/httpidlestate.h>
#include <SM/httpconnectionhandshakestate.h>
#include <SM/httpreadrequeststate.h>
#include <SM/httphandlerequeststate.h>
#include <SM/websocketrequeststate.h>

namespace stefanfrings{

HttpConnectionHandler::HttpConnectionHandler( QSettings* settings, HttpRequestHandler* requestHandler, QSslConfiguration* sslConfiguration, QObject *parent )
    : QObject(parent),
      m_type(UNDEFINED),
      m_serverName("Websocket Test Server"),
      socket(NULL),
      m_WebSocket(NULL),
      m_AllStates({ new SM::HttpIdleState("IDLE"),
                    new SM::HttpConnectionHandshakeState("CONNECT_HANDSHAKE"),
                    new SM::HttpReadRequestState("HTTP_GET_REQUEST"),
                    new SM::HttpHandleRequestState("HTTP_HANDLE_REQUEST"),
                    new SM::WebSocketRequestState("WEBSOCKET_REQUEST_STATE"),
                    /*TODO: TBD*/
                    new SM::ConnectionState("WEBSOCKET_HANDLING"),
                    new SM::ConnectionState("HTTP_ABORT"),
                    new SM::ConnectionState("CLOSE_CONNECTION"),
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
            socket->setReadBufferSize( 100000 );
            qDebug() << "In Buffer size: "  << socket->readBufferSize();
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
    connect(sslSocket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten(quint64)), Qt::QueuedConnection);
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
    m_CurrentConnectionState->handleConnectionEvent(*this, socketDescriptor);
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

}


void HttpConnectionHandler::disconnected()
{
#if defined SUPERVERBOSE
    qDebug("!!!!!!!!!!HttpConnectionHandler (%p) type(%d): disconnected", this,m_type);
#endif   
    m_CurrentConnectionState->disconnectEvent(*this);
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
            TODO: Towa !!!!
            QWebSocketHandshakeRequest wsRequest(pTcpSocket->peerPort(), isSecure );
            pWebSocket = QWebSocket::upgradeFrom( pTcpSocket , wsRequest, m_serverName );
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
}


bool HttpConnectionHandler::handlerSM()
{
    m_CurrentConnectionState->handlingLoopEvent(*this);
    return m_Dirty;
}

HttpConnectionHandler::HttpConnectionStateEnum HttpConnectionHandler::State() const
{
    return m_State;
}

void HttpConnectionHandler::setState(const HttpConnectionStateEnum &State)
{
    qDebug() << "HttpConnectionHandler change state to: " << State;
    m_State = State;
    m_CurrentConnectionState = m_AllStates[State];
}


void HttpConnectionHandler::websocketTextMessage( const QString & data)
{
    // Call the request mapper
    try
    {
        m_CurrentConnectionState->websocketTextMessageEvent( *this, data );
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
    }
    //  qDebug() << "Websocket: Reload timeout";
}

void HttpConnectionHandler::websocketbinaryFrameReceived(const QByteArray& data, bool final )
{
    // Call the request mapper
    try
    {
        m_CurrentConnectionState->websocketbinaryFrameReceivedEvent( *this, data, final );
    }
    catch (...)
    {
        qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",this);
    }
}

}
