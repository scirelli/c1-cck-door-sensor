#include "doorStateMachine.h"

/*
TODO: Current plan
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│           Pre-Idle                   │
│Animation: Faded blink pixel          │
│ * Shows sliding bar count down till  │
│  auto state change                   │
│Events:                               │
│ * auto-trans: after 5s               │
│ * btn-prs: next state idle           │
└──────────────────────────────────────┘
                    |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│               Idle                   │
│Animation: Pixel color warm blue      │
│  * Pixel color warm blue             │
│  * Prints Idle on screen or anim     │
│Events:                               │
│  * btn-prs: next state pre-newfile   │
└──────────────────────────────────────┘
                    |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│             Pre-NewFile              │
│Animation:                            │
│ * Fade blink red warning             │
│ * Prints a warning that of new file  │
│ * Shows sliding bar count down till  │
│  auto state change                   │
│Events:                               │
│  * auto-trans: state NewFile after 5s│
│  * btn-prs: next state record        │
└──────────────────────────────────────┘
                    |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│              NewFile                 │
│Animation:                            │
│ * Pixel color red warning            │
│ * Gives 3s count down until new file │
│  is created.                         │
│ * Prints new file name on display    │
│ * Shows sliding bar count down till  │
│  auto state change                   │
│Events:                               │
│ * auto-trans: state Pre-Record after │
│ new file creation and then an 5s     │
│ * btn-prs: next state Pre-Idle       │
└──────────────────────────────────────┘
                    |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│             Pre-Record               │
│Animation:                            │
│ * Pixel color light green            │
│ * Shows sliding bar count down till  │
│  auto state change                   │
│Events:                               │
│ * auto-trans: state Record after 3s  │
│ * btn-prs: next state Pre-Idle       │
└──────────────────────────────────────┘
                    |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│                Record                │
│Animation:                            │
│ * Graph data on display              │
│ * Max Gs recorded for current run    │
│Events:                               │
│ * btn-prs: next state Pre-Idle       │
└──────────────────────────────────────┘
*/
static door_state_container_t door_states[_DOOR_STATE_COUNT] = {
    [PRE_IDLE] = {
        .pre_idle = {
            .ds = {
                .base_state = {
                    .state_id = PRE_IDLE,
                    .animator_fnc = pre_idle_animator,
                    .enter_handler = pre_idle_enter,
                    .exit_handler = pre_idle_exit,
                    .evtHandler = door_state_event_handler,
                    //.next_state = (state_t*)&door_states[IDLE]
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
    door_state_t *state_ptr = door_get_state(state_id);
    if(!state_ptr || !is_valid_door_event_id(evt_id)) return false;

    state_ptr->event_handlers[evt_id] = fnc;

    return true;
}

bool door_set_animator_fnc(door_states_id_t state_id, stateAnimatorFnc_t fnc)
{
    door_state_t *state_ptr = door_get_state(state_id);
    if(!state_ptr || !fnc) return false;
    state_ptr->base_state.animator_fnc = fnc;
    return true;
}

bool door_set_enter_handle(door_states_id_t state_id, stateEnterHandler_t fnc)
{
    door_state_t *state_ptr = door_get_state(state_id);
    if(!state_ptr || !fnc) return false;
    state_ptr->base_state.enter_handler = fnc;
    return true;
}

bool door_set_exit_handle(door_states_id_t state_id, stateExitHandler_t fnc)
{
    door_state_t *state_ptr = door_get_state(state_id);
    if(!state_ptr || !fnc) return false;
    state_ptr->base_state.exit_handler = fnc;
    return true;
}

door_state_t* door_get_state(door_states_id_t id)
{
    if(!is_valid_door_state_id(id)) return NULL;
    return &door_states[id].door_state;
}

bool door_set_next_state(door_states_id_t id, door_state_t* s_ptr)
{
    if(!is_valid_door_state_id(id)) return false;
    door_states[id].generic.next_state = (state_t*)s_ptr;;
    return true;
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
static state_hndlr_status_t pre_idle_animator(state_t *s_ptr, cck_time_t t)
{
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_idle_enter(state_t *s_ptr, cck_time_t t)
{
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_idle_exit(state_t *s_ptr, cck_time_t t)
{
    return TRANSITION_OK;
}
//===================================================================



//===================================================================
// Idle State
//==================================================================
//===================================================================
