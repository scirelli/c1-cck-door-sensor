#ifndef _DOORSTATEMACHINE_H
#define _DOORSTATEMACHINE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <Arduino.h>
#include <STM32SD.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_NeoPixel.h>
#include <stdbool.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "cck_types.h"
#include "states.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define COLOR565(r, g, b) \
    ((uint16_t)(((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define DISPLAY_PRECISION   1

#define MAX_TRANSITION_TIME     10000L
#define MAX_PRE_IDLE_TIME       10000L
#define MAX_PRE_NEW_FILE_TIME   10000L
#define MAX_PRE_RECORD_TIME     10000L
#define NEW_FILE_MSG_DELAY      5000L
#define FILE_NAME "data_%lu.csv"

typedef struct door_state_t              door_state_t;
typedef struct door_pre_idle_state_t     door_pre_idle_state_t;
typedef struct door_idle_state_t         door_idle_state_t;
typedef struct door_pre_new_file_state_t door_pre_new_file_state_t;
typedef struct door_new_file_state_t     door_new_file_state_t;
typedef struct door_pre_record_state_t   door_pre_record_state_t;
typedef struct door_record_state_t       door_record_state_t;

typedef void (*door_event_handler_t)(door_state_t*, cck_time_t, void*);

//typedef void (*door_event_handler_fnc_t)(cck_time_t);
//typedef struct {
//   door_event_handler_fnc_t,
//   void *params;
//} door_event_handler_t;

#define DOOR_STATES \
    X(PRE_IDLE) \
    X(IDLE) \
    X(PRE_NEW_FILE) \
    X(NEW_FILE) \
    X(PRE_RECORD) \
    X(RECORD) \
    X(_DOOR_STATE_COUNT)

typedef enum:state_id_t {
#define X(name) name,
    DOOR_STATES
#undef X
} door_states_id_t;

/*
 * Events this door SM can handle.
 */
#define DOOR_EVENTS \
    X(DOOR_EVENT_BUTTON_1_PRESS) \
    X(DOOR_EVENT_BUTTON_2_PRESS) \
    X(DOOR_EVENT_BUTTON_3_PRESS) \
    X(DOOR_AUTO_TRANSITION) \
    X(DOOR_SENSOR_READING) \
    X(_DOOR_EVENT_COUNT)
typedef enum: state_event_id_t  {
#define X(name) name,
    DOOR_EVENTS
#undef X
} door_events_t;

struct door_state_t {
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
    bool fileCreated;
};

struct door_pre_record_state_t  {
    door_state_t ds;
};

struct door_record_state_t  {
    door_state_t ds;
    float max_gs;
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

typedef struct {
    const sensors_event_t *accel;
    const sensors_event_t *gyro;
    const sensors_event_t *mag;
    const sensors_event_t *temp;
} door_sensor_evt_ctx_t;

typedef struct {
   state_t* next_state;
} door_auto_evt_ctx_t;

typedef struct {
    const Adafruit_LSM6DSOX *lsm6ds;
    const Adafruit_LIS3MDL  *lis3mdl;
    File              *dataFile;
    Adafruit_NeoPixel *builtInNeo;
    Adafruit_SH1107   *display;
} door_sm_cfg_t;

typedef struct {
    state_machine_t sm;
    door_sm_cfg_t cfg;
} door_sm_t;

bool door_init_state_machine(door_sm_cfg_t);
bool door_run_state_machine(cck_time_t);
bool door_fire_event(state_event_id_t, cck_time_t, void* context=NULL);
bool door_set_event_handle(door_states_id_t, door_events_t, door_event_handler_t);
bool door_set_animator_fnc(door_states_id_t, stateAnimatorFnc_t);
bool door_set_enter_handle(door_states_id_t, stateEnterHandler_t);
bool door_set_exit_handle(door_states_id_t, stateExitHandler_t);
door_state_t* door_get_state(door_states_id_t);
bool door_set_next_state(door_states_id_t, door_state_t*);



static void door_state_event_handler(state_t* state_ptr, state_event_id_t evt, cck_time_t t, void*);
static bool is_valid_door_state_id(door_states_id_t);
static bool is_valid_door_event_id(door_events_t);
static void auto_evt_hndler(door_state_t *self, cck_time_t t, void *context);
static void fire_auto_transition_to(door_states_id_t s_id, cck_time_t t);
static void print_door_event_name(door_events_t evt_id);
static void print_state_name(door_states_id_t state_id);
static void print_state_name_every_x(state_t*, cck_time_t, cck_time_t x = 1000L);
static void display_default_settings();
static void log_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp);
static void write_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp);
static void display_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp);
static void blink_pixel(uint16_t hue, uint8_t sat, cck_time_t elapTime);
static void display_error(const char* errorMsg);
static void display_center(const char* str);
static void display_bot_center(const char *str);
static void display_top_center(const char *str);
static void time_bar(float);
static void halt();
static void halt_with_reason(const char[]);

// ==== Pre-Idle ====
static state_hndlr_status_t pre_idle_animator(state_t*, cck_time_t);
static state_hndlr_status_t pre_idle_enter(state_t*, cck_time_t);
static state_hndlr_status_t pre_idle_exit(state_t*, cck_time_t);
static void pre_idle_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context);
// ==================


// ==== Idle ====
static state_hndlr_status_t idle_animator(state_t*, cck_time_t);
static state_hndlr_status_t idle_enter(state_t*, cck_time_t);
static state_hndlr_status_t idle_exit(state_t*, cck_time_t);
static void idle_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context);
// ==================

// ==== PreNew File ====
static state_hndlr_status_t pre_new_file_animator(state_t*, cck_time_t);
static state_hndlr_status_t pre_new_file_enter(state_t*, cck_time_t);
static state_hndlr_status_t pre_new_file_exit(state_t*, cck_time_t);
static void pre_new_file_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context);
// =====================

// ==== New File ====
static state_hndlr_status_t new_file_animator(state_t*, cck_time_t);
static state_hndlr_status_t new_file_enter(state_t*, cck_time_t);
static state_hndlr_status_t new_file_exit(state_t*, cck_time_t);
static void new_file_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context);
// =====================

// ==== Pre-Record ====
static state_hndlr_status_t pre_record_animator(state_t*, cck_time_t);
static state_hndlr_status_t pre_record_enter(state_t*, cck_time_t);
static state_hndlr_status_t pre_record_exit(state_t*, cck_time_t);
static void pre_record_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context);
// =====================

// ==== Record ====
static state_hndlr_status_t record_animator(state_t*, cck_time_t);
static state_hndlr_status_t record_enter(state_t*, cck_time_t);
static state_hndlr_status_t record_exit(state_t*, cck_time_t);
static void record_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context);
static void record_sensor_evt_hndler(door_state_t *self, cck_time_t _, void *context);
// ================

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_DOORSTATEMACHINE_H
