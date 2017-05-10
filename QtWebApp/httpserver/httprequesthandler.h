/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include "httpglobal.h"
#include "httprequest.h"
#include "httpresponse.h"
#include"qwebsocket.h"
namespace stefanfrings {

/**
   The request handler generates a response for each HTTP request. Web Applications
   usually have one central request handler that maps incoming requests to several
   controllers (servlets) based on the requested path.
   <p>
   You need to override the service() method or you will always get an HTTP error 501.
   <p>
   @warning Be aware that the main request handler instance must be created on the heap and
   that it is used by multiple threads simultaneously.
   @see StaticFileController which delivers static local files.
*/

class DECLSPEC HttpRequestHandler : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(HttpRequestHandler)
public:
    typedef enum{
      FINISHED,
      WAIT,
      ERROR
    } ReqHandle_t;
    /**
     * Constructor.
     * @param parent Parent object.
     */
    HttpRequestHandler(QObject* parent=NULL);

    /** Destructor */
    virtual ~HttpRequestHandler();

    /**
      Generate a response for an incoming HTTP request.
      @param request The received HTTP request
      @param response Must be used to return the response
      @return  FINISHED - if handling is synchnonous and finish successfully
               WAIT  - if handling is assynchronous and service should be called again,
               in order to check finish status. For example if this service handler run some thread,
               in order to do some asynchronous work, then service handler should return WAIT. After that
               this handler will be called again to check for work finish and if it is  it will return status FINISHED.
               ERROR - On error
    */
    virtual ReqHandle_t service(HttpRequest& request, HttpResponse& response);
    virtual void websocketTextMessage( QWebSocket* ws, const QString & data);
    virtual void websocketbinaryFrameReceived( QWebSocket* ws, const QByteArray& data, bool final );
signals:
    void AsyncEvent();

};

} // end of namespace

#endif // HTTPREQUESTHANDLER_H
