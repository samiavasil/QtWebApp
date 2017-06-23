#ifndef PROTORESPONSE_H
#define PROTORESPONSE_H

#include"protorequest.h"
class ProtoResponse
{
public:
    typedef enum{
        IDLE,
        WAIT,
        ABORT,
        COMPLETE,
        STATUS_NUM
    }ResponseStatus;

    ProtoResponse();
    ResponseStatus GetStatus();
    virtual ResponseStatus CreateResponse( ProtoRequest& Request ) = 0;
    virtual void ResetResponse( )  = 0;
protected:
    ResponseStatus m_Status;
};

#endif // PROTORESPONSE_H
