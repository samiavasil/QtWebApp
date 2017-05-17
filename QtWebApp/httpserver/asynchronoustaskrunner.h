#ifndef ASYNCHRONOUSTASKRUNNER_H
#define ASYNCHRONOUSTASKRUNNER_H
#include<QThread>

class Task{
public:
    virtual bool RunTask() = 0;
};

class AsynchronousTaskRunner:public QThread{
    Q_OBJECT
public:
    typedef enum{
        OK,
        NOK,
        BUSY
    }ATR_T;
    AsynchronousTaskRunner( Task* task = NULL, QObject *parent = Q_NULLPTR );
    ~AsynchronousTaskRunner();
    virtual void StartRun();
    virtual void StopRun();
    bool IsRunning();
    ATR_T AttachTask( Task* task );

protected slots:
    void taskFinished();
signals:
    void AsynchronousTaskFinished();
protected:
    virtual void run();
protected:
    Task* m_Task;
    bool m_Running;
};

#endif // ASYNCHRONOUSTASKRUNNER_H
