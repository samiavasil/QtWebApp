/**
  @file
  @author Stefan Frings
*/

#include <QCoreApplication>
#include "requestmapper.h"
#include "filelogger.h"
#include "staticfilecontroller.h"


/** Redirects log messages to a file */
extern FileLogger* logger;

/** Controller for static files */
extern StaticFileController* staticFileController;

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent)
{
    qDebug("RequestMapper: created");
    connect(&m_FileUploadController,SIGNAL(AsyncEvent()),this,SIGNAL(AsyncEvent()));
}


RequestMapper::~RequestMapper()
{
    qDebug("RequestMapper: deleted");
}


HttpRequestHandler::ReqHandle_t RequestMapper::service(HttpRequest& request, HttpResponse& response)
{
    HttpRequestHandler::ReqHandle_t ret = HttpRequestHandler::ERROR;
    QByteArray path=request.getPath();
   // qDebug("RequestMapper: path=%s",path.data());

    // For the following pathes, each request gets its own new instance of the related controller.

    if (path.startsWith("/dump"))
    {
        ret = m_DumpController.service(request, response);
    }

    else if (path.startsWith("/template"))
    {
        ret = m_TemplateController.service(request, response);
    }

    else if (path.startsWith("/form"))
    {
        ret = m_FormController.service(request, response);
    }

    else if (path.startsWith("/file"))
    {
        ret = m_FileUploadController.service(request, response);
    }

    else if (path.startsWith("/session"))
    {
        ret = m_SessionController.service(request, response);
    }

    // All other pathes are mapped to the static file controller.
    // In this case, a single instance is used for multiple requests.
    else
    {
        ret = staticFileController->service(request, response);
    }

 //   qDebug("RequestMapper: finished request");

    // Clear the log buffer
    if (logger)
    {
       logger->clear();
    }
    return ret;
}

QTcpSocket* serviceConnection;

void RequestMapper::websocketTextMessage(QWebSocket *ws, const QString &data)
{
    int t = 123;
    ws->sendTextMessage(data);
    switch(  t ){
    case 0:{
        serviceConnection = new QTcpSocket(this);
        break;
    }

    default:{

        break;
    }
    }
}

void RequestMapper::websocketbinaryFrameReceived(QWebSocket *ws, const QByteArray &data, bool final)
{
    qCritical("DEMO1 HttpRequestHandler:  websocketbinaryFrameReceived TBI");
    qDebug("DEMO1 HttpRequestHandler:websocketbinaryFrameReceived TBI");
    if( final || !final )/*Remove warning*/
        ws->sendBinaryMessage(data);
}
