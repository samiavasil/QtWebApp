#ifndef PROTOREQUEST_H
#define PROTOREQUEST_H
#include "commchannel.h"

class ProtoRequest
{
public:

    typedef enum{
        IDLE,
        WAIT,
        ABORT,
        COMPLETE,
        STATUS_NUM
    }RequestStatus;
    ProtoRequest();
    RequestStatus GetStatus();
    virtual RequestStatus ReadRequest( CommChannel& Com ) = 0;
    virtual void ResetRequest() = 0;
protected:
    RequestStatus m_Status;
};

#endif // PROTOREQUEST_H
