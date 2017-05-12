#ifndef STATEMACHINE_H
#define STATEMACHINE_H


class StateMachine
{
public:
   typedef enum{
        IDLE,
        BUSY
    }StateMachine_t;

    virtual  StateMachine::StateMachine_t  StateMachineHandler() = 0;
};

#endif // STATEMACHINE_H
