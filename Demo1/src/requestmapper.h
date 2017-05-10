/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httprequesthandler.h"
#include "staticfilecontroller.h"
#include "controller/dumpcontroller.h"
#include "controller/templatecontroller.h"
#include "controller/formcontroller.h"
#include "controller/fileuploadcontroller.h"
#include "controller/sessioncontroller.h"


using namespace stefanfrings;

/**
  The request mapper dispatches incoming HTTP requests to controller classes
  depending on the requested path.
*/

class RequestMapper : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)
public:

    /**
      Constructor.
      @param parent Parent object
    */
    RequestMapper(QObject* parent=0);

    /**
      Destructor.
    */
    ~RequestMapper();

    /**
      Dispatch incoming HTTP requests to different controllers depending on the URL.
      @param request The received HTTP request
      @param response Must be used to return the response
    */
    ReqHandle_t service(HttpRequest& request, HttpResponse& response);

    void websocketTextMessage( QWebSocket* ws, const QString & data);

    void websocketbinaryFrameReceived( QWebSocket* ws, const QByteArray& data, bool final );

protected:
    DumpController        m_DumpController;
    TemplateController    m_TemplateController;
    FormController        m_FormController;
    FileUploadController  m_FileUploadController;
    SessionController     m_SessionController;

};

#endif // REQUESTMAPPER_H
