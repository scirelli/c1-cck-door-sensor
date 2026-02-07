#include "doorStateMachine.h"

static state_t state_pre_idle;
static state_t state_idle;

state_machine_t door_state_machine = {
    .cur_state = &state_idle
};

//===================================================================
// Pre Idle State
//==================================================================
static void state_pre_idle_handler(state_t *this, cck_time_t t)
{
}
static void state_pre_idle_event_handler(state_t* this, state_event_t evt, cck_time_t t)
{
}
static state_t state_pre_idle = {
    .state_id = STATE_PRE_IDLE,
    .evtHandler = state_pre_idle_event_handler,
    .event_action = NULL,
    .enter_handler = NULL,
    .exit_handler = NULL,
    .next_state = &state_idle
};
//===================================================================



//===================================================================
// Idle State
//==================================================================
door_event_handler_t idle_event_handlers[_DOOR_EVENT_COUNT];
static void state_idle_event_handler(state_t* this, state_event_t evt, cck_time_t t)
{
    switch(evt) {
        case DOOR_EVENT_BUTTON_1_PRESS:
            if(idle_event_handlers[DOOR_EVENT_BUTTON_1_PRESS]) idle_event_handlers[DOOR_EVENT_BUTTON_1_PRESS](t);
        break;
    }
}
static state_t state_idle = {
    .state_id = STATE_IDLE,
    .evtHandler = state_idle_event_handler,
    .event_action = NULL,
    .enter_handler = NULL,
    .exit_handler = NULL,
    .next_state = NULL
};
//===================================================================
