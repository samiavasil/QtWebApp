/**
  @file
  @author Stefan Frings
*/

#ifndef FILEUPLOADCONTROLLER_H
#define FILEUPLOADCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"
#include<QThread>
using namespace stefanfrings;

class ExecutiveTask:public QThread{
    Q_OBJECT
public:
    ExecutiveTask(QTemporaryFile * File, QObject *parent = Q_NULLPTR );
    ~ExecutiveTask();

protected:
    virtual void run();
signals:
    /* This signal should be connected to slot which should be called from main event loop (QueuedConnection) and there will
     * call a real socket write fnction. Qt sockets can't be used directly from this context because it is a diferent thread
     * ( Qt doesn't allow usage of QtcpSocket from thread
       from one where is creation thread ).
    */
    void write(QSharedPointer<QByteArray>);
protected:
      QTemporaryFile* m_File;
};

/**
  This controller displays a HTML form for file upload and recieved the file.
*/

class FileUploadController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(FileUploadController)
public:

    /** Constructor */
    FileUploadController();

    /** Generates the response */
    ReqHandle_t service(HttpRequest& request, HttpResponse& response);
protected:
    bool m_Finished;
    ExecutiveTask* m_CurrentTask;
    HttpResponse* m_response;
protected slots:
    void finished();
    /*Write to socket slot*/
    void write(QSharedPointer<QByteArray> Bytes);

};



#endif // FILEUPLOADCONTROLLER_H
