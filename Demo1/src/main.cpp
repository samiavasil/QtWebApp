/**
  @file
  @author Stefan Frings test
*/

#include <QCoreApplication>
#include <QDir>
#include "httplistener.h"
#include "templatecache.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "filelogger.h"
#include "requestmapper.h"

using namespace stefanfrings;

/** Cache for template files */
TemplateCache* templateCache;

/** Storage for session cookies */
HttpSessionStore* sessionStore;

/** Controller for static files */
StaticFileController* staticFileController;

/** Redirects log messages to a file */
FileLogger* logger;


/** Search the configuration file */
QString searchConfigFile()
{
    QString binDir=QCoreApplication::applicationDirPath();
    QString appName=QCoreApplication::applicationName();
    QString fileName(appName+".ini");

    QStringList searchList;
    searchList.append(binDir);
    searchList.append(binDir+"/etc");
    searchList.append(binDir+"/../etc");
    searchList.append(binDir+"/../../etc"); // for development without shadow build
    searchList.append(binDir+"/../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(QDir::rootPath()+"etc/opt");
    searchList.append(QDir::rootPath()+"etc");

    foreach (QString dir, searchList)
    {
        QFile file(dir+"/"+fileName);
        if (file.exists())
        {
            // found
            fileName=QDir(file.fileName()).canonicalPath();
            qDebug("Using config file %s",qPrintable(fileName));
            return fileName;
        }
    }

    // not found
    foreach (QString dir, searchList)
    {
        qWarning("%s/%s not found",qPrintable(dir),qPrintable(fileName));
    }
    qFatal("Cannot find config file %s",qPrintable(fileName));
    return 0;
}


#include<QThreadPool>
#include<QRunnable>
#include<QElapsedTimer>

class Test:public QRunnable
{
public:
    Test(){
        m_cur = cur;
        cur++;
    }


    virtual void run(){
//        QThread::msleep( 2000);
//        for( int i=0;i<300;i++){
//           QThread::msleep( 100);
//           qDebug() << "Test " << m_cur;
//        }
        qDebug() << "Run Task " << m_cur;
        QFile file("C:\\Projects\\GitHub\\QtWebApp\\build-Demo1-Desktop_Qt_5_7_0_MinGW_32bit-Debug\\debug\\test.zip");

        if (!file.open(QIODevice::ReadOnly )){
            qDebug() << "Can't open file in: " << file.fileName();
            return;
        }
        QFile fileout( QString("C:\\Projects\\GitHub\\QtWebApp\\build-Demo1-Desktop_Qt_5_7_0_MinGW_32bit-Debug\\debug\\test%1.zip").arg(m_cur) );
        if (!fileout.open(QIODevice::WriteOnly )){
            qDebug() << "Can't open out file : " << fileout.fileName();
            return;
        }
        while (!file.atEnd()) {
            fileout.write( file.read(1000000) );
        }
        file.close();
        fileout.close();
        qDebug() << "Finish Task " << m_cur;
    }
protected:
   static int cur;
   int m_cur;
};
int Test::cur;

void threadPollTesting(){
    QThreadPool::globalInstance()->setMaxThreadCount(1000);
    QElapsedTimer timer;
    qint64 mSec;
    timer.start();



    qDebug() << "ThreadPool task started: ";
    qDebug() << "MaxThreads " << QThreadPool::globalInstance()->maxThreadCount();
    for( int i=0; i < 300; i++){
        if( !QThreadPool::globalInstance()->tryStart( new Test()  ) ){
            qDebug() << "Can't run Task " << i;
        }
    }
    QThreadPool::globalInstance()->waitForDone(-1);

    mSec = timer.elapsed();

    qDebug() << "ThreadPool task finished: ";
    qDebug() << "Time for execution[sec]: " << ((double)mSec)/1000.0;
}

/**
  Entry point of the program.
*/
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);

    app.setApplicationName("Demo1");
    app.setOrganizationName("Butterfly");

    // Find the configuration file
    QString configFileName=searchConfigFile();
threadPollTesting();
    // Configure logging into a file
    /*
    QSettings* logSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    logSettings->beginGroup("logging");
    FileLogger* logger=new FileLogger(logSettings,10000,&app);
    logger->installMsgHandler();
    */

    // Configure template loader and cache
    QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    templateSettings->beginGroup("templates");
    templateCache=new TemplateCache(templateSettings,&app);

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    sessionSettings->beginGroup("sessions");
    sessionStore=new HttpSessionStore(sessionSettings,&app);

    // Configure static file controller
    QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    fileSettings->beginGroup("docroot");
    staticFileController=new StaticFileController(fileSettings,&app);

    // Configure and start the TCP listener
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    listenerSettings->beginGroup("listener");
    new HttpListener(listenerSettings,new RequestMapper(&app),&app);

    qWarning("Application has started");

    app.exec();

    qWarning("Application has stopped");
}
