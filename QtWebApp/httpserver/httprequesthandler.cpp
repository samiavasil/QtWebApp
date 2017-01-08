/**
  @file
  @author Stefan Frings
*/

#include "httprequesthandler.h"

using namespace stefanfrings;

HttpRequestHandler::HttpRequestHandler(QObject* parent)
    : QObject(parent)
{}

HttpRequestHandler::~HttpRequestHandler()
{}

void HttpRequestHandler::service(HttpRequest& request, HttpResponse& response)
{
    qCritical("HttpRequestHandler: you need to override the service() function");
    qDebug("HttpRequestHandler: request=%s %s %s",request.getMethod().data(),request.getPath().data(),request.getVersion().data());
    response.setStatus(501,"not implemented");
    response.write("501 not implemented",true);
}

void HttpRequestHandler::websocketTextMessage(QWebSocket *ws, const QString & data )
{
    qCritical("HttpRequestHandler: you need to override the websocketTextMessage function");
    qDebug("HttpRequestHandler: Default websocket echo implementation. You need to override the websocketTextMessage function");
    ws->sendTextMessage(data);
}

void HttpRequestHandler::websocketbinaryFrameReceived(QWebSocket *ws, const QByteArray& data, bool final )
{
    qCritical("HttpRequestHandler: you need to override the websocketbinaryFrameReceived function");
    qDebug("HttpRequestHandler: Default websocket echo implementation. You need to override the websocketbinaryFrameReceived function");
    ws->sendBinaryMessage(data);
}
