#include "states.h"


bool state_init_machine(state_machine_t *sm_ptr, state_t *s_ptr)
{
    if(!sm_ptr || !s_ptr) return false;
    sm_ptr->cur_state = s_ptr;
    if(s_ptr->enter_handler) s_ptr->enter_handler(s_ptr, 0);
    return true;
}

state_proc_status_t state_machine_run(state_machine_t *sm_ptr, cck_time_t t)
{
    if(!sm_ptr || !sm_ptr->cur_state) return STATE_PROC_ERROR;
    if(!sm_ptr->cur_state->animator_fnc) return STATE_PROC_SUCCESS;

    state_proc_status_t status = STATE_PROC_UNKNOWN;

    switch(sm_ptr->cur_state->animator_fnc(sm_ptr->cur_state, t)) {
        case ACTION_SUCCESS:
            status = STATE_PROC_SUCCESS;
            break;
        case ACTION_FAILURE:
            status = STATE_PROC_ERROR;
    }

    return status;
}

bool state_fire_event(state_machine_t *sm_ptr, state_event_id_t evt, cck_time_t t, void* context)
{
    if(sm_ptr && sm_ptr->cur_state) {
        if(sm_ptr->cur_state->evtHandler) {
            sm_ptr->cur_state->evtHandler(sm_ptr->cur_state, evt, t, context);
        }
        if(sm_ptr->cur_state->next_state) {
            state_transition(sm_ptr, sm_ptr->cur_state->next_state, t);
        }
    }
    return true;
}




static state_trans_status_t state_transition(state_machine_t *sm_ptr, state_t *next_state_ptr, cck_time_t t)
{
    if (!sm_ptr || !next_state_ptr) {
        return TRANSITION_FAIL;
    }

    if (sm_ptr->cur_state->exit_handler) sm_ptr->cur_state->exit_handler(sm_ptr->cur_state, t);

    sm_ptr->cur_state = next_state_ptr;

    if (sm_ptr->cur_state->enter_handler) sm_ptr->cur_state->enter_handler(sm_ptr->cur_state,  t);

    return TRANSITION_SUCCESS;
}
