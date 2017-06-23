#ifndef PROTOCOL_H
#define PROTOCOL_H

#include"commchannel.h"

class ProtoRequest;
class ProtoResponse;

class Protocol
{
public:
    typedef enum{
        IDLE,
        GET_REQUEST,
        HANDLE_REQUEST,
        SEND_RESPONSE,
        ABORT_REQUEST,
        ABORT_RESPONSE,
        NUM_STATES
    } ProtoState;
    ProtoState GetState();
    virtual bool ProtoHandleLoop();
protected:
    ProtoRequest*   m_Request;
    ProtoResponse*  m_Response;
    CommChannel*    m_Com;
    ProtoState      m_State;
};

#endif // PROTOCOL_H
