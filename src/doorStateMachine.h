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

#define STATE_PAUSE                -1
#define STATE_ERROR                 0
#define STATE_PRE_IDLE              1
#define STATE_IDLE                  2
#define STATE_PRE_NEW_FILE          3
#define STATE_NEW_FILE              4
#define STATE_PRE_READ_SENSORS      5
#define STATE_READ_SENSORS          6
#define STATE_TEST                  9

#define STATE_ERROR_NONE 20
#define STATE_ERROR_TEST 21

typedef void (*door_event_handler_t)(cck_time_t);

typedef enum: state_event_t  {
    DOOR_EVENT_BUTTON_1_PRESS = 0,
    _DOOR_EVENT_COUNT
} door_events_t;

extern state_machine_t  door_state_machine;
extern door_event_handler_t idle_event_handlers[_DOOR_EVENT_COUNT];

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_DOORSTATEMACHINE_H
