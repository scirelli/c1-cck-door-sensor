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
static door_sm_t door_sm;

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
                },
            }
        }
    },
    [IDLE] = {
        .idle = {
            .ds = {
                .base_state = {
                    .state_id = IDLE,
                    .animator_fnc = idle_animator,
                    .evtHandler = door_state_event_handler,
                    .next_state = NULL
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = idle_btn1_prs_hndler
                }
            },
        }
    }
};

//===================================================================
// Door SM State functions
//==================================================================
bool door_init_state_machine(door_sm_cfg_t config)
{
    door_sm.cfg = config;
    return state_init_machine(&door_sm.sm, &door_states[IDLE].generic);
}

bool door_run_state_machine(cck_time_t t)
{
    state_machine_run(&door_sm.sm, t);
    return true; //TODO: implement
}

bool door_fire_event(state_event_id_t evt_id, cck_time_t t, void *context)
{
    return state_fire_event(&door_sm.sm, evt_id, t, context);
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

static void door_state_event_handler(state_t* state_ptr, state_event_id_t evt_id, cck_time_t t, void* context)
{
    if(!state_ptr) return;
    if(!is_valid_door_event_id((door_events_t)evt_id)) return;
    door_state_t* self = (door_state_t*)state_ptr;

    if(self->event_handlers[evt_id]) {
        self->event_handlers[evt_id](self, t, context);
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
static state_hndlr_status_t idle_animator(state_t *self, cck_time_t _)
{
    door_sm.cfg.builtInNeo->clear();
    door_sm.cfg.builtInNeo->setPixelColor(0, Adafruit_NeoPixel::Color(0, 63, 0) );
    door_sm.cfg.builtInNeo->show();
    return TRANSITION_OK;
}

static void idle_btn1_prs_hndler(door_state_t *self, cck_time_t _, void *context)
{
}
//===================================================================



//===================================================================
// Utils
//==================================================================
static void log_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp)
{
  // Display the results (acceleration is measured in m/s^2)
  Serial.print("\t\tAccel X: ");
  Serial.print(accel->acceleration.x, 4);
  Serial.print(" \tY: ");
  Serial.print(accel->acceleration.y, 4);
  Serial.print(" \tZ: ");
  Serial.print(accel->acceleration.z, 4);
  Serial.println(" \tm/s^2 ");

  // Display the results (rotation is measured in rad/s)
  Serial.print("\t\tGyro  X: ");
  Serial.print(gyro->gyro.x, 4);
  Serial.print(" \tY: ");
  Serial.print(gyro->gyro.y, 4);
  Serial.print(" \tZ: ");
  Serial.print(gyro->gyro.z, 4);
  Serial.println(" \tradians/s ");

  // Display the results (magnetic field is measured in uTesla)
  Serial.print(" \t\tMag   X: ");
  Serial.print(mag->magnetic.x, 4);
  Serial.print(" \tY: ");
  Serial.print(mag->magnetic.y, 4);
  Serial.print(" \tZ: ");
  Serial.print(mag->magnetic.z, 4);
  Serial.println(" \tuTesla ");

  Serial.print("\t\tTemp   :\t\t\t\t\t");
  Serial.print(temp->temperature);
  Serial.println(" \tdeg C");
  Serial.println();
}

static void write_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp)
{
    if (door_sm.cfg.dataFile) {
        // Accel X m/s^2
        door_sm.cfg.dataFile->print(accel->acceleration.x, 4); door_sm.cfg.dataFile->print(",");
        // Accel Y m/s^2
        door_sm.cfg.dataFile->print(accel->acceleration.y, 4); door_sm.cfg.dataFile->print(",");
        // Accel Z m/s^2
        door_sm.cfg.dataFile->print(accel->acceleration.z, 4); door_sm.cfg.dataFile->print(",");
        // Gyro  X rad/s
        door_sm.cfg.dataFile->print(gyro->gyro.x, 4); door_sm.cfg.dataFile->print(",");
        // Gyro Y rad/s
        door_sm.cfg.dataFile->print(gyro->gyro.y, 4); door_sm.cfg.dataFile->print(",");
        // Gyro Z rad/s
        door_sm.cfg.dataFile->print(gyro->gyro.z, 4);

        door_sm.cfg.dataFile->flush();
    } else {
        Serial.print("error on file handle");
    }
}

static void display_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp)
{
    Adafruit_SH1107 display = *door_sm.cfg.display;
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  // Display the results (acceleration is measured in m/s^2)
  display.print("AX: ");                                     // 3
  display.print(accel->acceleration.x, DISPLAY_PRECISION);  // 3
  display.print(" Y: ");                                    // 4
  display.print(accel->acceleration.y, DISPLAY_PRECISION);  // 3
  display.print(" Z: ");                                    // 4
  display.println(accel->acceleration.z, DISPLAY_PRECISION);  // 3
  //display.println(" m/s^2 ");                               // 7

  // Display the results (rotation is measured in rad/s)
  display.print("GX: ");
  display.print(gyro->gyro.x, DISPLAY_PRECISION);
  display.print(" Y: ");
  display.print(gyro->gyro.y, DISPLAY_PRECISION);
  display.print(" Z: ");
  display.println(gyro->gyro.z, DISPLAY_PRECISION);
  //display.println(" r/s");

  // Display the results (magnetic field is measured in uTesla)
  display.print("MX: ");
  display.print(mag->magnetic.x, DISPLAY_PRECISION);
  display.print(" Y: ");
  display.print(mag->magnetic.y, DISPLAY_PRECISION);
  display.print(" Z: ");
  display.println(mag->magnetic.z, DISPLAY_PRECISION);
  //display.println(" uT ");

  display.print("Temp: ");
  display.print(temp->temperature);
  display.print(" C");

  display.display();
}
//===================================================================
