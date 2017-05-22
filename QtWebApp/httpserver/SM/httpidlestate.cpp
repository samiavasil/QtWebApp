#include "httpidlestate.h"

namespace stefanfrings {
void HttpIdleState::handleConnectionEvent(stefanfrings::HttpConnectionHandler &conHndl, const stefanfrings::tSocketDescriptor &socketDescriptor)
{

#if defined SUPERVERBOSE
    qDebug("HttpConnectionHandler (%p): handle new connection", this);
#endif
    conHndl.busy = true;
    Q_ASSERT(conHndl.socket->isOpen()==false); // if not, then the handler is already busy
    //UGLY workaround - we need to clear writebuffer before reusing this socket
    //https://bugreports.qt-project.org/browse/QTBUG-28914
   conHndl.socket->connectToHost("",0);
   conHndl.socket->abort();

#ifndef QT_NO_OPENSSL
    if( conHndl.sslConfiguration )
    {
        QSslSocket* sslSocket = NULL;
        if( conHndl.socket )
        {
            sslSocket = qobject_cast<QSslSocket*>(conHndl.socket);
            Q_ASSERT( sslSocket );
        }
        else
        {
            sslSocket = new QSslSocket();
            Q_ASSERT( sslSocket );
        }
        sslSocket->setSslConfiguration(*conHndl.sslConfiguration);
        conHndl.socket=sslSocket;
        QObject::connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)),&conHndl, SLOT(sslErrors(QList<QSslError>)) );
        QObject::connect(sslSocket, SIGNAL(encrypted()), &conHndl, SLOT(encrypted()));
        QObject::connect(sslSocket, SIGNAL(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator*)),
        &conHndl, SLOT(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator*)));
#if defined SUPERVERBOSE
        qDebug("HttpConnectionHandler (%p): SSL is enabled", this);
#endif
    }
    else
    {
        // Connect signals
        QObject::connect( conHndl.socket, SIGNAL(readyRead()), &conHndl,SLOT(readyRead()), Qt::QueuedConnection );
        QObject::connect( conHndl.socket, SIGNAL(disconnected()), &conHndl,SLOT(disconnected()) );
    }
#else
   // Connect signals
   QObject::connect( conHndl.socket, SIGNAL(readyRead()), this,SLOT(readyRead()), Qt::QueuedConnection );
   QObject::connect( conHndl.socket, SIGNAL(disconnected()), this,SLOT(disconnected()) );
#endif
    if( !conHndl.socket->setSocketDescriptor(socketDescriptor) )
    {
        qCritical("HttpConnectionHandler (%p): cannot initialize socket: %s", this,qPrintable(conHndl.socket->errorString()));
        conHndl.disconnected();
        return;
    }

    #ifndef QT_NO_OPENSSL
        // Switch on encryption, if SSL is configured
        if (conHndl.sslConfiguration)
        {
#if defined SUPERVERBOSE
            qDebug("HttpConnectionHandler (%p): Starting encryption", this);
#endif
            QSslSocket* ssl_socket = (QSslSocket*)conHndl.socket;
            ssl_socket->ignoreSslErrors();
            ssl_socket->startServerEncryption();
        }
    #endif
    QObject::connect( &conHndl, SIGNAL(signalExecuteSM()), &conHndl, SLOT(handlerSM()), Qt::QueuedConnection );
    // Start timer for read timeout
    int readTimeout=conHndl.settings->value("readTimeout",10000).toInt();
    conHndl.readTimer.start(readTimeout);
    // delete previous request
    delete conHndl.currentRequest;
    conHndl.currentRequest=0;
    delete conHndl.currentResponse;
    conHndl.currentResponse = 0;
    conHndl.setState( conHndl.CONNECT_HANDSHAKE_STATE );
}
}
