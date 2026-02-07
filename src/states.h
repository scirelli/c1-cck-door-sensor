#ifndef _STATES_H
#define _STATES_H
#include <stdbool.h>
#include "Arduino.h"
#include "cck_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    TRANSITION_SUCCESS,
    TRANSITION_FAIL
} state_trans_status_t;

typedef enum {
    TRANSITION_NEXT,
    TRANSITION_STAY,
    TRANSITION_ERROR,
} state_hndlr_status_t;

typedef enum {
    STATE_PROC_UNKNOWN,
    STATE_PROC_SUCCESS,
    STATE_PROC_ERROR,
} state_proc_status_t;

typedef enum {
    ACTION_SUCCESS,
    ACTION_FAILURE
} state_evt_act_status_t;

typedef int8_t state_id_t;
typedef int8_t state_event_t;
typedef struct state_t state_t;

/*
 * stateEventAction_t:
 * Will either immediately handle the event and set the next state.
 * Or not set the next state but set the event action to run. This action is meant to be run each frame indefinitely while in the state.
 * The idea is event action is something that runs over time or takes a long time to run. Say fading pixels.
 * I will have to think this through as action will need to keep it's state.
 */
typedef state_evt_act_status_t (*stateEventAction_t)(state_t*, cck_time_t);
/*
 * stateEnterHandler_t
 * Function to allow something to happen when entering a state. Maybe use for something like auto-actions/auto-event.
 * Need to think this through more.
 */
typedef state_hndlr_status_t (*stateEnterHandler_t)(state_t*, cck_time_t); //Status currently ignored
/*
 * stateExitHandler_t
 * Can be used to and cleanup that needs to be done when exiting a state.
 */
typedef state_hndlr_status_t (*stateExitHandler_t)(state_t*, cck_time_t); //Status currently ignored
/*
 * stateEventHandler_t
 * sets the state's event handler. This handler will be called when events are fired on the state machine.
 */
typedef void (*stateEventHandler_t)(state_t*, state_event_t, cck_time_t);

struct state_t{
    state_id_t state_id;
    cck_time_t enter_time;
    stateEventAction_t event_action;
    stateEnterHandler_t enter_handler;
    stateExitHandler_t exit_handler;
    stateEventHandler_t evtHandler;
    state_t *next_state;
};

typedef struct {
    state_t *cur_state;
} state_machine_t;

static state_trans_status_t state_transition(state_machine_t *, state_t *next_state, cck_time_t t);

state_proc_status_t state_machine_run(state_machine_t *, cck_time_t);
bool state_fire_event(state_machine_t *, state_event_t, cck_time_t);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_STATES_H
