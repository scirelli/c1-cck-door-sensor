#include "states.h"


static state_trans_status_t state_transition(state_machine_t *sm, state_t *next_state, cck_time_t t)
{
    if (!sm || !next_state) {
        return TRANSITION_FAIL;
    }

    if (sm->cur_state->exit_handler) sm->cur_state->exit_handler(sm->cur_state, t);

    sm->cur_state = next_state;

    if (sm->cur_state->enter_handler) sm->cur_state->enter_handler(sm->cur_state,  t);

    return TRANSITION_SUCCESS;
}

state_proc_status_t state_machine_run(state_machine_t *sm, cck_time_t t)
{
    if(!sm || !sm->cur_state)  return STATE_PROC_ERROR;
    if(!sm->cur_state->event_action) return STATE_PROC_SUCCESS;

    state_proc_status_t status = STATE_PROC_UNKNOWN;

    switch(sm->cur_state->event_action(sm->cur_state, t)) {
        case ACTION_SUCCESS:
            status = STATE_PROC_SUCCESS;
            break;
        case ACTION_FAILURE:
            status = STATE_PROC_ERROR;
    }

    return status;
}

bool state_fire_event(state_machine_t *sm, state_event_t evt, cck_time_t t)
{
    if(sm && sm->cur_state){
        sm->cur_state->evtHandler(sm->cur_state, evt, t);
        if(sm->cur_state->next_state) {
            state_transition(sm, sm->cur_state->next_state, t);
        }
    }
}
