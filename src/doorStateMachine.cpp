#include "doorStateMachine.h"

static door_state_container_t door_states[_DOOR_STATE_COUNT] = {
    [PRE_IDLE] = {
        .pre_idle = {
            .ds = {
                .base_state = {
                    .state_id = PRE_IDLE,
                    .evtHandler = door_state_event_handler,
                    .next_state = (state_t*)&door_states[IDLE]
                }
            }
        }
    },
    [IDLE] = {
        .idle = {
            .ds = {
                .base_state = {
                    .state_id = IDLE,
                    .evtHandler = door_state_event_handler,
                    .next_state = NULL
                }
            }
        }
    }
};

//===================================================================
// Door SM State functions
//==================================================================
bool setup_door_state_machine(state_machine_t *sm_ptr)
{
    if(!sm_ptr) return false;
    return state_init_machine(sm_ptr, &door_states[IDLE].generic);
}

bool door_set_event_handle(door_states_id_t state_id, door_events_t evt_id, door_event_handler_t fnc)
{
    door_state_t *state_ptr = get_door_state(state_id);
    if(!state_ptr || !is_valid_door_event_id(evt_id)) return false;

    state_ptr->event_handlers[evt_id] = fnc;

    return true;
}

bool door_set_animator_fnc(door_states_id_t state_id, stateAnimatorFnc_t fnc)
{
    door_state_t *state_ptr = get_door_state(state_id);
    if(!state_ptr || !fnc) return false;
    state_ptr->base_state.animator_fnc = fnc;
    return true;
}

bool door_set_enter_handle(door_states_id_t state_id, stateEnterHandler_t fnc)
{
    door_state_t *state_ptr = get_door_state(state_id);
    if(!state_ptr || !fnc) return false;
    state_ptr->base_state.enter_handler = fnc;
    return true;
}

bool door_set_exit_handle(door_states_id_t state_id, stateExitHandler_t fnc)
{
    door_state_t *state_ptr = get_door_state(state_id);
    if(!state_ptr || !fnc) return false;
    state_ptr->base_state.exit_handler = fnc;
    return true;
}

door_state_t* get_door_state(door_states_id_t id)
{
    if(!is_valid_door_state_id(id)) return NULL;
    return &door_states[id].door_state;
}
//===================================================================

//===================================================================
// Door SM state help functions
//==================================================================
static bool is_valid_door_state_id(door_states_id_t state_id)
{
    return (state_id >= 0 && state_id < _DOOR_STATE_COUNT);
}

static bool is_valid_door_event_id(door_events_t evt_id)
{
    return (evt_id >= 0 && evt_id < _DOOR_EVENT_COUNT);
}

static void door_state_event_handler(state_t* state_ptr, state_event_id_t evt_id, cck_time_t t)
{
    if(!state_ptr) return;
    if(!is_valid_door_event_id((door_events_t)evt_id)) return;
    door_state_t* self = (door_state_t*)state_ptr;

    if(self->event_handlers[evt_id]) {
        self->event_handlers[evt_id](self, t);
    }
}
//===================================================================


//===================================================================
// Pre Idle State
//==================================================================
//===================================================================



//===================================================================
// Idle State
//==================================================================
//===================================================================
