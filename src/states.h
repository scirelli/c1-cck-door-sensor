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
    TRANSITION_OK,
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

//Use to give you states IDs
typedef int8_t state_id_t;
// State event types, create your own types and use it's ID to map to a function.
typedef int8_t state_event_id_t;
//Forward declaration of a basic state struct.
typedef struct state_t state_t;

/*
 * stateAnimatorFnc_t:
 * Will either immediately handle the event and set the next state.
 * Or not set the next state but set the event action to run. This action is meant to be run each frame indefinitely while in the state.
 * The idea is event action is something that runs over time or takes a long time to run. Say fading pixels.
 * I will have to think this through as action will need to keep it's state.
 */
typedef state_hndlr_status_t (*stateAnimatorFnc_t)(state_t*, cck_time_t);
/*
 * stateEnterHandler_t
 * Function to allow something to happen when entering a state. Maybe use for something like auto-actions/auto-event.
 * Need to think this through more.
 */
typedef state_hndlr_status_t (*stateEnterHandler_t)(state_t*, cck_time_t);
/*
 * stateExitHandler_t
 * Can be used to and cleanup that needs to be done when exiting a state.
 */
typedef state_hndlr_status_t (*stateExitHandler_t)(state_t*, cck_time_t);
/*
 * stateEventHandler_t
 * Sets the state's event handler. This handler will be called when events are fired on the state machine.
 * Use this function to map state_event_id_t to functions.
 */
typedef void (*stateEventHandler_t)(state_t*, state_event_id_t, cck_time_t, void*);

struct state_t{
    state_id_t state_id;
    cck_time_t enter_time;
    stateAnimatorFnc_t animator_fnc;
    stateEnterHandler_t enter_handler;
    stateExitHandler_t exit_handler;
    stateEventHandler_t evtHandler;
    state_t *next_state;
};

typedef struct {
    state_t *cur_state;
} state_machine_t;

static state_trans_status_t state_transition(state_machine_t *, state_t *next_state, cck_time_t t);

/*
 * state_init_machine
 * Initialize the state machine with it's first state.
 */
bool state_init_machine(state_machine_t*, state_t *);
/*
 * state_machine_run
 * This function goes in the main loop, allows states' action functions continuously.
 * TODO: Will refactor this so that the state table can call registered functions on timeouts or intervals.
 */
state_proc_status_t state_machine_run(state_machine_t *, cck_time_t);
/*
 * state_fire_event
 * Fires and event on the state machine, allowing the current state to process events.
 */
bool state_fire_event(state_machine_t *, state_event_id_t, cck_time_t, void* context);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_STATES_H
