/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPCONNECTIONHANDLER_H
#define HTTPCONNECTIONHANDLER_H

#ifndef QT_NO_OPENSSL
   #include <QSslConfiguration>
#endif
#include <QTcpSocket>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include "httpglobal.h"
#include "httprequest.h"
#include "httprequesthandler.h"
#include "qwebsocket.h"

namespace stefanfrings {

/** Alias type definition, for compatibility to different Qt versions */
#if QT_VERSION >= 0x050000
    typedef qintptr tSocketDescriptor;
#else
    typedef int tSocketDescriptor;
#endif

/** Alias for QSslConfiguration if OpenSSL is not supported */
#ifdef QT_NO_OPENSSL
  #define QSslConfiguration QObject
#endif

/**
  The connection handler accepts incoming connections and dispatches incoming requests to to a
  request mapper. Since HTTP clients can send multiple requests before waiting for the response,
  the incoming requests are queued and processed one after the other.
  <p>
  Example for the required configuration settings:
  <code><pre>
  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000
  </pre></code>
  <p>
  The readTimeout value defines the maximum time to wait for a complete HTTP request.
  @see HttpRequest for description of config settings maxRequestSize and maxMultiPartSize.
*/
class DECLSPEC HttpConnectionHandler :
#if defined(HANDLER_THREADING)
        public QThread
#else
        public QObject
#endif
{
    Q_OBJECT
    Q_DISABLE_COPY(HttpConnectionHandler)

public:
    typedef enum{
        IDLE,
        CONNECT_HANDSHAKE,
        HTTP_GET_REQUEST,
        HTTP_HANDLE_REQUEST,
        WEBSOCKET_HANDLING,
        HTTP_ABORT,
        CLOSE_CONNECTION,
    } HttpConnectionState;
    /**
      Constructor.
      @param settings Configuration settings of the HTTP webserver
      @param requestHandler Handler that will process each incoming HTTP request
      @param sslConfiguration SSL (HTTPS) will be used if not NULL
    */
    HttpConnectionHandler(QSettings* settings, HttpRequestHandler* requestHandler,QSslConfiguration* sslConfiguration=NULL,QObject *parent = Q_NULLPTR );

    /** Destructor */
    virtual ~HttpConnectionHandler();

    /** Returns true, if this handler is in use. */
    bool isBusy();

    /** Mark this handler as busy */
    void setBusy();

private:
    typedef enum{
        UNDEFINED,
        HTTP,
        WEBSOCKET
    } HandlerType_t;

    HandlerType_t m_type;

    QString m_serverName;

    /** Configuration settings */
    QSettings* settings;

    /** TCP socket of the current connection  */
    QTcpSocket* socket;

    QWebSocket* m_WebSocket;

    /** Time for read timeout detection */
    QTimer readTimer;

    /** Storage for the current incoming HTTP request */
    HttpRequest* currentRequest;

    HttpResponse* currentResponse;

    /** Dispatches received requests to services */
    HttpRequestHandler* requestHandler;

    /** This shows the busy-state from a very early time */
    bool busy;

    /** Configuration for SSL */
    QSslConfiguration* sslConfiguration;

    HttpConnectionState m_State;

    /** Executes the threads own event loop */
    void run();

    /**  Create SSL or TCP socket */
    void createSocket();

    bool websocketHandshake(QTcpSocket *pTcpSocket);

signals:
    void dellMeTestSignal();
public slots:

    /**
      Received from from the listener, when the handler shall start processing a new connection.
      @param socketDescriptor references the accepted connection.
    */
    void handleConnection(tSocketDescriptor socketDescriptor);

protected:
    HttpConnectionState readHttpRequest();

    HttpConnectionHandler::HttpConnectionState handleHttpRequest();

    HttpConnectionHandler::HttpConnectionState httpAbort();
private slots:

    /** Received from the socket when a read-timeout occured */
    void readTimeout();

    /** Received from the socket when incoming data can be read */
    void handlerSM();

    void websocketTextMessage(const QString &data);

    void websocketbinaryFrameReceived(const QByteArray &data, bool final );

    /** Received from the socket when a connection has been closed */
    void disconnected();

    void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator);
    void encrypted();
    void sslErrors(const QList<QSslError> &errors);
};

} // end of namespace

#endif // HTTPCONNECTIONHANDLER_H
