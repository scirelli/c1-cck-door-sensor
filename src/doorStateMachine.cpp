#include "doorStateMachine.h"

static door_sm_t door_sm;
static const char* doorStateNames[] = {
#define X(name) #name,
    DOOR_STATES
#undef X
};
const char* doorEventNames[] = {
#define X(name) #name,
    DOOR_EVENTS
#undef X
};


static door_state_container_t door_states[_DOOR_STATE_COUNT] = {
    [PRE_IDLE] = {
        .pre_idle = {
            .ds = {
                .base_state = {
                    .state_id = PRE_IDLE,
                    .animator_fnc = pre_idle_animator,
                    .enter_handler = pre_idle_enter,
                    .exit_handler = pre_idle_exit,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = pre_idle_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL,
                    [DOOR_AUTO_TRANSITION] = door_auto_evt_hndler
                }
            }
        }
    },
    [IDLE] = {
        .idle = {
            .ds = {
                .base_state = {
                    .state_id = IDLE,
                    .animator_fnc = idle_animator,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = idle_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL
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
    return state_init_machine(&door_sm.sm, &door_states[PRE_IDLE].generic);
}

bool door_run_state_machine(cck_time_t t)
{
    state_machine_run(&door_sm.sm, t);
    return true; //TODO: implement
}

bool door_fire_event(state_event_id_t evt_id, cck_time_t t, void *context)
{
    if(!is_valid_door_event_id((door_events_t)evt_id)) return false;
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

static void door_auto_evt_hndler(door_state_t *self, cck_time_t t, void *context)
{
    if(!context) return;
    door_auto_evt_ctx_t *evt = (door_auto_evt_ctx_t*)context;
    self->base_state.next_state = evt->next_state;
}
//===================================================================


//===================================================================
// Pre Idle State
//==================================================================
static state_hndlr_status_t pre_idle_animator(state_t *self_ptr, cck_time_t t)
{
    //TODO: Blink neopixel for 5s before moving to idle
    print_state_name_every_x(self_ptr, t);
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_idle_enter(state_t *self_ptr, cck_time_t t)
{
    //TODO: init state data.
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_idle_exit(state_t *self_ptr, cck_time_t t)
{
    //TODO: Clean up state data.
    return TRANSITION_OK;
}
static void pre_idle_btn1_prs_hndler(door_state_t *self_ptr, cck_time_t t, void *context)
{
    door_state_t *ds = door_get_state(IDLE);
    if(!ds) return;
    door_auto_evt_ctx_t d_evt = {
        .next_state = &ds->base_state
    };
    door_fire_event(DOOR_AUTO_TRANSITION, t, &d_evt);
}
//===================================================================



//===================================================================
// Idle State
//==================================================================
static state_hndlr_status_t idle_animator(state_t *self_ptr, cck_time_t t)
{
    door_sm.cfg.builtInNeo->clear();
    door_sm.cfg.builtInNeo->setPixelColor(0, Adafruit_NeoPixel::Color(0, 31, 0) );
    door_sm.cfg.builtInNeo->show();
    print_state_name_every_x(self_ptr, t);
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
static void print_state_name(door_states_id_t state_id)
{
    if(!is_valid_door_state_id(state_id)) {
        Serial.println("Invalid state id");
    }else {
        Serial.println(doorStateNames[state_id]);
    }
}
static void print_door_event_name(door_events_t evt_id)
{
    if(!is_valid_door_event_id(evt_id)) {
        Serial.println("Invalid door event id");
    }else {
        Serial.println(doorEventNames[evt_id]);
    }
}

static void print_state_name_every_x(state_t *s_ptr, cck_time_t t, cck_time_t x)
{
    static unsigned long prevTime = 0;
    unsigned long elapTime = t - prevTime;
    if(elapTime > x) {
        prevTime = t;
        Serial.print("In ");
        Serial.print(doorStateNames[s_ptr->state_id]);
        Serial.println(" state");
    }
}
//===================================================================
