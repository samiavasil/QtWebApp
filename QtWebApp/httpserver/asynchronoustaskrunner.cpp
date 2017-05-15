#include "asynchronoustaskrunner.h"
#include<QDebug>

AsynchronousTaskRunner::AsynchronousTaskRunner( Task* task, QObject *parent ):QThread(parent),m_Task(task)
{
    m_Running = false;
    connect( this, SIGNAL(finished()),this,SLOT(taskFinished()) );
}

AsynchronousTaskRunner::~AsynchronousTaskRunner()
{
    if( m_Task ){
        delete m_Task;
    }
}

bool AsynchronousTaskRunner::IsRunning()
{
    return QThread::isRunning();
}

void AsynchronousTaskRunner::StartRun()
{
    m_Running = true;
    start();
}

void AsynchronousTaskRunner::StopRun()
{
    //TODO: implement  this as some async signal
    qDebug() << "Stop AsynchronousTaskRunner";
    terminate();//TODO:
    if( !QThread::wait(10000) ){
        qDebug() << "Bad news: AsynchronousTaskRunner doesn't stop fo 100 sec ' ";
    }
    m_Running = false;
}

AsynchronousTaskRunner::ATR_T AsynchronousTaskRunner::AttachTask( Task *task )
{
    ATR_T Res = NOK;
    if( m_Running ){
        Res = BUSY;
    }else{
        if( m_Task )
        m_Task = task;
    }
    return Res;
}

void AsynchronousTaskRunner::taskFinished()
{
    m_Running = false;
}

void AsynchronousTaskRunner::run()
{
    if( m_Task ){
        while( m_Task->RunTask())yieldCurrentThread();
    }
}
