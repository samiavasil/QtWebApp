#include "protocol.h"
#include "protorequest.h"
#include "protoresponse.h"

Protocol::ProtoState Protocol::GetState()
{
    return m_State;
}

bool Protocol::ProtoHandleLoop()
{
    bool Ret = false;
    bool Continue = false;
    if( 0 != m_Com ){
        do{
            Continue = false;
            switch(m_State){
            case IDLE:{
                break;
            }
            case GET_REQUEST:{
                ProtoRequest::RequestStatus Status = m_Request->ReadRequest(*m_Com);
                if( ProtoRequest::ABORT == Status ){
                    m_State = ABORT_REQUEST;
                }else if( ProtoRequest::COMPLETE == Status ){
                    m_State = HANDLE_REQUEST;
                    Continue = true;
                }
                break;
            }
            case HANDLE_REQUEST:{
                ProtoResponse::ResponseStatus Status = m_Response->CreateResponse( *m_Request );
                if( ProtoResponse::ABORT == Status ){
                    m_State = ABORT_RESPONSE;
                }else if( ProtoResponse::COMPLETE == Status ){
                    m_State = SEND_RESPONSE;
                    Continue = true;
                }
                break;
            }
            case SEND_RESPONSE:{
                m_Com->SendData();
                if( 0 ){
                    m_State = SEND_RESPONSE;
                    Continue = true;
                }
                break;
            }
            case ABORT_RESPONSE:{
                //TODO: TBD
                m_State = IDLE;
                break;
            }
            case ABORT_REQUEST:{
                //TODO: TBD
                m_State = IDLE;
                break;
            }
            default :{
                break;
            }
            }
        }while(Continue);
    }
    return Ret;
}
