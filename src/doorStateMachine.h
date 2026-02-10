#ifndef _DOORSTATEMACHINE_H
#define _DOORSTATEMACHINE_H
#include <stdbool.h>
#include <stddef.h>
#include "Arduino.h"
#include "cck_types.h"
#include "states.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct door_state_t              door_state_t;
typedef struct door_pre_idle_state_t     door_pre_idle_state_t;
typedef struct door_idle_state_t         door_idle_state_t;
typedef struct door_pre_new_file_state_t door_pre_new_file_state_t;
typedef struct door_new_file_state_t     door_new_file_state_t;
typedef struct door_pre_record_state_t   door_pre_record_state_t;
typedef struct door_record_state_t       door_record_state_t;

typedef void (*door_event_handler_t)(door_state_t*, cck_time_t);

//typedef void (*door_event_handler_fnc_t)(cck_time_t);
//typedef struct {
//   door_event_handler_fnc_t,
//   void *params;
//} door_event_handler_t;

typedef enum:state_id_t {
    PRE_IDLE = 0,
    IDLE,
    PRE_NEW_FILE,
    NEW_FILE,
    PRE_RECORD,
    RECORD,
    _DOOR_STATE_COUNT
} door_states_id_t;

/*
 * Events this door SM can handle.
 */
typedef enum: state_event_id_t  {
    DOOR_EVENT_BUTTON_1_PRESS = 0,
    AUTO_TRANSITION,
    _DOOR_EVENT_COUNT
} door_events_t;

struct  door_state_t {
    state_t base_state;
    door_event_handler_t event_handlers[_DOOR_EVENT_COUNT];
};

struct door_pre_idle_state_t {
    door_state_t ds;
};

struct door_idle_state_t {
    door_state_t ds;
};

struct door_pre_new_file_state_t {
    door_state_t ds;
};

struct door_new_file_state_t  {
    door_state_t ds;
};

struct door_pre_record_state_t  {
    door_state_t ds;
};

struct door_record_state_t  {
    door_state_t ds;
};

typedef union {
    state_t                   generic;
    door_state_t              door_state;
    door_pre_idle_state_t     pre_idle;
    door_idle_state_t         idle;
    door_pre_new_file_state_t pre_new_file;
    door_new_file_state_t     new_file;
    door_pre_record_state_t   pre_record;
    door_record_state_t       record;
} door_state_container_t;



bool setup_door_state_machine(state_machine_t *);
bool door_set_event_handle(door_states_id_t, door_events_t, door_event_handler_t);
bool door_set_animator_fnc(door_states_id_t, stateAnimatorFnc_t);
bool door_set_enter_handle(door_states_id_t, stateEnterHandler_t);
bool door_set_exit_handle(door_states_id_t, stateExitHandler_t);
door_state_t* door_get_state(door_states_id_t);
bool door_set_next_state(door_states_id_t, door_state_t*);



static void door_state_event_handler(state_t* state_ptr, state_event_id_t evt, cck_time_t t);
static bool is_valid_door_state_id(door_states_id_t);
static bool is_valid_door_event_id(door_events_t);



// ==== Pre-Idle ====
static state_hndlr_status_t pre_idle_animator(state_t*, cck_time_t);
static state_hndlr_status_t pre_idle_enter(state_t*, cck_time_t);
static state_hndlr_status_t pre_idle_exit(state_t*, cck_time_t);
// ==================




#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_DOORSTATEMACHINE_H
