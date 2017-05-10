/**
  @file
  @author Stefan Frings
*/

#include "fileuploadcontroller.h"
#include<QSharedPointer>

ExecutiveTask::ExecutiveTask(QTemporaryFile* File , QObject *parent ):QThread(parent),m_File(File)
{
    qRegisterMetaType<QSharedPointer<QByteArray>>("QSharedPointer<QByteArray>");
}

ExecutiveTask::~ExecutiveTask()
{

}

void ExecutiveTask::run()
{
//    //m_Response.setHeader("Content-Type", "image/jpeg");
//    QTemporaryFile* file=m_Request.getUploadedFile("file1");
    qDebug() << "FileName: " << m_File->fileName();
    QSharedPointer<QByteArray> buffer;
    if (m_File)
    {
        while (!m_File->atEnd() && !m_File->error())
        {
            buffer = QSharedPointer<QByteArray>( new QByteArray(m_File->read(65536)) );
            emit write(buffer);
        }
    }
    else
    {
        buffer = QSharedPointer<QByteArray>( new QByteArray( ("upload failed") ) );
        emit write(buffer);
    }

}



FileUploadController::FileUploadController()
{
    m_CurrentTask = NULL;
     m_Finished   = false;
}

void Function( HttpRequest* request, HttpResponse* response)
{
    response->setHeader("Content-Type", "image/jpeg");
    QTemporaryFile* file=request->getUploadedFile("file1");
    qDebug() << "FileName: " << file->fileName();
    if (file)
    {
        while (!file->atEnd() && !file->error())
        {
            QByteArray buffer=file->read(65536);
            response->write(buffer);
        }
    }
    else
    {
        response->write("upload failed");
    }
}

void FileUploadController::finished()
{
    m_CurrentTask->deleteLater();
    m_CurrentTask = NULL;
    m_Finished = true;
    emit AsyncEvent();
}

void FileUploadController::write(QSharedPointer<QByteArray> Bytes)
{
    m_response->write(*Bytes.data());
}

HttpRequestHandler::ReqHandle_t FileUploadController::service(HttpRequest& request, HttpResponse& response)
{
    static int state = 0;
    HttpRequestHandler::ReqHandle_t ret = HttpRequestHandler::ERROR;
    switch (state) {
    case 0:
    {
        m_Finished = true;
        if (request.getParameter("action")=="show")
        {
            //TODO
            m_response = &response;
            m_CurrentTask = new ExecutiveTask(request.getUploadedFile("file1"),this);
#if 0
            m_response->setHeader("Content-Type", "image/jpeg");
#else
            m_response->setHeader("Content-Type","application/force-download");
            //      m_response->setContentLength((int)f.length());
            //response.setContentLength(-1);
            m_response->setHeader("Content-Transfer-Encoding", "binary");
            m_response->setHeader("Content-Disposition","attachment; filename=\"test.bin\"");
#endif

            connect( m_CurrentTask,SIGNAL(finished()),this, SLOT(finished()) );
            connect( m_CurrentTask,SIGNAL(write(QSharedPointer<QByteArray>)),this, SLOT(write(QSharedPointer<QByteArray>)) );
            m_CurrentTask->start();
            //Function( &request, &response);
            m_Finished = false;
            state = 1;
            ret = HttpRequestHandler::WAIT;
        }
        else
        {
            response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
            response.write("<html><body>");
            response.write("Upload a JPEG image file<p>");
            response.write("<form method=\"post\" enctype=\"multipart/form-data\">");
            response.write("  <input type=\"hidden\" name=\"action\" value=\"show\">");
            response.write("  File: <input type=\"file\" name=\"file1\"><br>");
            response.write("  <input type=\"submit\">");
            response.write("</form>");
            response.write("</body></html>",true);
            ret = HttpRequestHandler::FINISHED;
        }
        break;
    }
    case 1:
    {
        if( m_Finished )
        {
            ret = HttpRequestHandler::FINISHED;
            state = 0;
        }
        else
        {
            ret = HttpRequestHandler::WAIT;
        }

        break;
    }
    default:
        break;
    }

    return ret;
}

