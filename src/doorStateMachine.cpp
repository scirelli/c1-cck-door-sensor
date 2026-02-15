#include "doorStateMachine.h"

static door_sm_t door_sm;
static File dataFile;
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
                    [DOOR_AUTO_TRANSITION] = auto_evt_hndler
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
                    .enter_handler = idle_enter,
                    .exit_handler = idle_exit,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = idle_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL,
                    [DOOR_AUTO_TRANSITION] = auto_evt_hndler
                }
            },
        }
    },
    [PRE_NEW_FILE] = {
        .pre_new_file = {
            .ds = {
                .base_state = {
                    .state_id = PRE_NEW_FILE,
                    .animator_fnc = pre_new_file_animator,
                    .enter_handler = pre_new_file_enter,
                    .exit_handler = pre_new_file_exit,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = pre_new_file_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL,
                    [DOOR_AUTO_TRANSITION] = auto_evt_hndler
                }
            },
        }
    },
    [NEW_FILE] = {
        .new_file = {
            .ds = {
                .base_state = {
                    .state_id = NEW_FILE,
                    .animator_fnc = new_file_animator,
                    .enter_handler = new_file_enter,
                    .exit_handler = new_file_exit,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = new_file_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL,
                    [DOOR_AUTO_TRANSITION] = auto_evt_hndler
                }
            },
            .fileCreated = false
        }
    },
    [PRE_RECORD] = {
        .pre_record = {
            .ds = {
                .base_state = {
                    .state_id = PRE_RECORD,
                    .animator_fnc = pre_record_animator,
                    .enter_handler = pre_record_enter,
                    .exit_handler = pre_record_exit,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = pre_record_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL,
                    [DOOR_AUTO_TRANSITION] = auto_evt_hndler
                }
            },
        }
    },
    [RECORD] = {
        .record = {
            .ds = {
                .base_state = {
                    .state_id = RECORD,
                    .animator_fnc = record_animator,
                    .enter_handler = record_enter,
                    .exit_handler = record_exit,
                    .evtHandler = door_state_event_handler
                },
                .event_handlers = {
                    [DOOR_EVENT_BUTTON_1_PRESS] = record_btn1_prs_hndler,
                    [DOOR_EVENT_BUTTON_2_PRESS] = NULL,
                    [DOOR_EVENT_BUTTON_3_PRESS] = NULL,
                    [DOOR_AUTO_TRANSITION] = auto_evt_hndler,
                    [DOOR_SENSOR_READING] = record_sensor_evt_hndler
                }
            },
            .max_gs = 0.0f
        }
    }
};

//===================================================================
// Door SM State functions
//===================================================================
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
    door_states[id].generic.next_state = (state_t*)s_ptr;
    return true;
}
//===================================================================

//===================================================================
// Door SM state help functions
//===================================================================
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

static void auto_evt_hndler(door_state_t *self, cck_time_t t, void *context)
{
    if(!context) return;
    door_auto_evt_ctx_t *evt = (door_auto_evt_ctx_t*)context;
    self->base_state.next_state = evt->next_state;
}
//===================================================================



//===================================================================
// PreIdle State
//===================================================================
static state_hndlr_status_t pre_idle_animator(state_t *self_ptr, cck_time_t curTime)
{
    cck_time_t elapTime = curTime - self_ptr->enter_time;

    print_state_name_every_x(self_ptr, curTime);

    if(elapTime > MAX_PRE_IDLE_TIME) {
        fire_auto_transition_to(IDLE, curTime);
        return TRANSITION_NEXT;
    }

    blink_pixel(43690, 255, elapTime);

    door_sm.cfg.display->clearDisplay();
    door_sm.cfg.display->setTextSize(2);
    //TODO: Convert this to something like frames of animation
    if(elapTime > 0 && elapTime < 500) {
        display_center("T");
    } else if(elapTime >= 500 && elapTime < 1000) {
        display_center("I");
    } else if(elapTime >= 1000 && elapTime < 1500) {
        display_center("GA   ");
    } else if(elapTime >= 1500 && elapTime < 2000) {
        display_center("GA GA");
    } else if(elapTime >= 2000 && elapTime < 3000) {
        display_center("EERRR!!");
    } else if(elapTime >= 3000 && elapTime < MAX_PRE_IDLE_TIME) {
        display_center("TIGGER!!");
    }
    door_sm.cfg.display->setTextSize(1);
    display_bot_center("Pre-Idle");

    time_bar((float)elapTime/MAX_PRE_IDLE_TIME);

    door_sm.cfg.display->display();
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_idle_enter(state_t *self_ptr, cck_time_t t)
{
    display_default_settings();
    self_ptr->enter_time = t;
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_idle_exit(state_t *self_ptr, cck_time_t t)
{
    door_sm.cfg.builtInNeo->setPixelColor(0, door_sm.cfg.builtInNeo->Color(0,0,0));
    door_sm.cfg.builtInNeo->show();
    return TRANSITION_OK;
}
static void pre_idle_btn1_prs_hndler(door_state_t *self_ptr, cck_time_t t, void *context)
{
    fire_auto_transition_to(IDLE, t);
}
//===================================================================



//===================================================================
// Idle State
//===================================================================
static state_hndlr_status_t idle_animator(state_t *self_ptr, cck_time_t t)
{
    print_state_name_every_x(self_ptr, t);
    return TRANSITION_OK;
}
static state_hndlr_status_t idle_enter(state_t *self_ptr, cck_time_t t)
{
    self_ptr->enter_time = t;
    door_sm.cfg.builtInNeo->clear();
    door_sm.cfg.builtInNeo->setPixelColor(0,
        door_sm.cfg.builtInNeo->gamma32(
            door_sm.cfg.builtInNeo->ColorHSV(43690, 255, 128)
        )
    );
    door_sm.cfg.builtInNeo->show();

    display_default_settings();
    display_top_center("Last run max Gs");
    door_record_state_t *ds_ptr = (door_record_state_t*)door_get_state(RECORD);
    if(ds_ptr) {
        const char *str1 = "16.00";
        const char* str2 = " m/s^2";
        int16_t x1, y1, x2, y2;
        uint16_t w1, h1, w2, h2;


        door_sm.cfg.display->setTextSize(2);
        door_sm.cfg.display->getTextBounds(str1, 0, 0, &x1, &y1, &w1, &h1);
        door_sm.cfg.display->setTextSize(1);
        door_sm.cfg.display->getTextBounds(str2, 0, 0, &x2, &y2, &w2, &h2);

        door_sm.cfg.display->setCursor(
                door_sm.cfg.display->width()/2 - (w1/2 + w2/2),
                door_sm.cfg.display->height()/2 - h1/2
        );
        door_sm.cfg.display->setTextSize(2);
        door_sm.cfg.display->print(ds_ptr->max_gs);

        door_sm.cfg.display->setCursor(
                (door_sm.cfg.display->width()/2 - (w1/2 + w2/2)) + w1/2 + w2*.7f,
                door_sm.cfg.display->height()/2 - h1/2 + h2
        );
        door_sm.cfg.display->setTextSize(1);
        door_sm.cfg.display->print(str2);

        display_bot_center("Idle");
    }
    door_sm.cfg.display->display();

    return TRANSITION_OK;
}
static state_hndlr_status_t idle_exit(state_t *self_ptr, cck_time_t t)
{
    return TRANSITION_OK;
}
static void idle_btn1_prs_hndler(door_state_t *self, cck_time_t t, void *context)
{
    fire_auto_transition_to(PRE_NEW_FILE, t);
}
//===================================================================



//===================================================================
// Pre-New File State
//===================================================================
static state_hndlr_status_t pre_new_file_animator(state_t *self_ptr, cck_time_t curTime)
{
    cck_time_t elapTime = curTime - self_ptr->enter_time;
    print_state_name_every_x(self_ptr, curTime);

    if(elapTime > MAX_PRE_NEW_FILE_TIME) {
        fire_auto_transition_to(NEW_FILE, curTime);
        return TRANSITION_NEXT;
    }

    blink_pixel(0, 255, elapTime);

    door_sm.cfg.display->clearDisplay();
    door_sm.cfg.display->setTextSize(2);
    display_center("Create a  new file?");
    door_sm.cfg.display->setTextSize(1);
    display_bot_center("Press btn to cancel");
    time_bar((float)elapTime/MAX_PRE_NEW_FILE_TIME);
    door_sm.cfg.display->display();
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_new_file_enter(state_t *self_ptr, cck_time_t t)
{
    self_ptr->enter_time = t;
    door_sm.cfg.builtInNeo->clear();
    door_sm.cfg.builtInNeo->setPixelColor(0,
        door_sm.cfg.builtInNeo->gamma32(
            door_sm.cfg.builtInNeo->ColorHSV(0, 255, 128)
        )
    );
    door_sm.cfg.builtInNeo->show();

    display_default_settings();
    return TRANSITION_OK;
}
static state_hndlr_status_t pre_new_file_exit(state_t *self_ptr, cck_time_t t)
{
    Serial.println("Pre-New File Exit");
    return TRANSITION_OK;
}
static void pre_new_file_btn1_prs_hndler(door_state_t *self, cck_time_t t, void *context)
{
    fire_auto_transition_to(IDLE, t);
}
//===================================================================



//===================================================================
// New File State
//===================================================================
static state_hndlr_status_t new_file_animator(state_t *self_ptr, cck_time_t curTime)
{
    cck_time_t elapTime = curTime - self_ptr->enter_time;
    print_state_name_every_x(self_ptr, curTime);

    blink_pixel(0, 255, elapTime);
    if(((door_new_file_state_t*)self_ptr)->fileCreated) {
        if(elapTime > NEW_FILE_MSG_DELAY) {
            fire_auto_transition_to(PRE_RECORD, curTime);
        }
    }
    return TRANSITION_OK;
}
static state_hndlr_status_t new_file_enter(state_t *self_ptr, cck_time_t t)
{
    self_ptr->enter_time = t;
    door_new_file_state_t *ds_ptr = (door_new_file_state_t*)self_ptr;
    ds_ptr->fileCreated = false;

    door_sm.cfg.builtInNeo->clear();
    door_sm.cfg.builtInNeo->setPixelColor(0,
        door_sm.cfg.builtInNeo->gamma32(
            door_sm.cfg.builtInNeo->ColorHSV(0, 255, 128)
        )
    );
    door_sm.cfg.builtInNeo->show();

    display_default_settings();
    door_sm.cfg.display->setTextSize(2);
    display_center("Creating a new file");
    door_sm.cfg.display->display();

    char filename[20];
    snprintf(filename, sizeof(filename), FILE_NAME, t);
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.seek(dataFile.size()); //Move to the end of the file for appending.
        ds_ptr->fileCreated = true;
    } else {
        Serial.print("Error opening file ");
        Serial.println(filename);
        halt();
    }
    door_sm.cfg.display->clearDisplay();
    display_center("New file  created");
    door_sm.cfg.display->setTextSize(1);
    display_top_center(filename);
    display_bot_center("New file");
    door_sm.cfg.display->display();
    return TRANSITION_OK;
}
static state_hndlr_status_t new_file_exit(state_t *self_ptr, cck_time_t t)
{
    Serial.println("New File Exit");
    return TRANSITION_OK;
}
static void new_file_btn1_prs_hndler(door_state_t *self, cck_time_t t, void *context)
{
    fire_auto_transition_to(PRE_RECORD, t);
}
//===================================================================



//===================================================================
// Pre-Record State
//===================================================================
static state_hndlr_status_t pre_record_animator(state_t *self_ptr, cck_time_t curTime)
{
    cck_time_t elapTime = curTime - self_ptr->enter_time;
    print_state_name_every_x(self_ptr, curTime);

    if(elapTime > MAX_PRE_RECORD_TIME) {
        fire_auto_transition_to(RECORD, curTime);
        return TRANSITION_NEXT;
    }

    blink_pixel(21845, 255, elapTime);

    door_sm.cfg.display->clearDisplay();
    door_sm.cfg.display->setTextSize(1);
    display_bot_center("Press btn to start");
    door_sm.cfg.display->setTextSize(2);
    display_top_center("Get ready to record!");
    time_bar((float)elapTime/MAX_PRE_RECORD_TIME);
    door_sm.cfg.display->display();

    return TRANSITION_OK;
}
static state_hndlr_status_t pre_record_enter(state_t *self_ptr, cck_time_t t)
{
    self_ptr->enter_time = t;
    door_sm.cfg.builtInNeo->clear();
    door_sm.cfg.builtInNeo->setPixelColor(0,
        door_sm.cfg.builtInNeo->gamma32(
            door_sm.cfg.builtInNeo->ColorHSV(21845, 255, 128)
        )
    );
    door_sm.cfg.builtInNeo->show();
    display_default_settings();

    return TRANSITION_OK;
}
static state_hndlr_status_t pre_record_exit(state_t *self_ptr, cck_time_t t)
{
    Serial.println("Pre-Record Exit");
    return TRANSITION_OK;
}
static void pre_record_btn1_prs_hndler(door_state_t *self, cck_time_t t, void *context)
{
    fire_auto_transition_to(RECORD, t);
}
//===================================================================



//===================================================================
// Record State
//===================================================================
static state_hndlr_status_t record_animator(state_t *self_ptr, cck_time_t curTime)
{
    cck_time_t elapTime = curTime - self_ptr->enter_time;
    print_state_name_every_x(self_ptr, curTime);
    blink_pixel(21845, 255, elapTime);
    return TRANSITION_OK;
}
static state_hndlr_status_t record_enter(state_t *self_ptr, cck_time_t t)
{
    self_ptr->enter_time = t;
    door_record_state_t *ds_ptr = (door_record_state_t*)self_ptr;
    ds_ptr->max_gs = 0.0f;

    display_default_settings();
    display_bot_center("Press btn to stop");
    door_sm.cfg.display->setTextSize(2);
    display_center("Recording...");
    door_sm.cfg.display->display();

    return TRANSITION_OK;
}
static state_hndlr_status_t record_exit(state_t *self_ptr, cck_time_t t)
{
    if(dataFile) {
        dataFile.close();
    }
    return TRANSITION_OK;
}
static void record_btn1_prs_hndler(door_state_t *self, cck_time_t t, void *context)
{
    fire_auto_transition_to(IDLE, t);
}
static void record_sensor_evt_hndler(door_state_t *self, cck_time_t t, void *context)
{
    if(!context) return;
    door_sensor_evt_ctx_t *evt = (door_sensor_evt_ctx_t*)context;
    door_record_state_t *ds_ptr = (door_record_state_t*)self;

    float accel_mag = sqrtf(
        evt->accel->acceleration.x * evt->accel->acceleration.x +
        evt->accel->acceleration.y * evt->accel->acceleration.y +
        evt->accel->acceleration.z * evt->accel->acceleration.z
    );
    float gs = accel_mag / 9.81f;
    if(gs > ds_ptr->max_gs) {
        ds_ptr->max_gs = gs;
    }

    write_sensor_data(evt->accel, evt->gyro, evt->mag, evt->temp);
    display_sensor_data(evt->accel, evt->gyro, evt->mag, evt->temp);
}
//===================================================================


//===================================================================
// Utils
//===================================================================
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
    if (dataFile) {
        // Accel X m/s^2
        dataFile.print(accel->acceleration.x, 4); dataFile.print(",");
        // Accel Y m/s^2
        dataFile.print(accel->acceleration.y, 4); dataFile.print(",");
        // Accel Z m/s^2
        dataFile.print(accel->acceleration.z, 4); dataFile.print(",");
        // Gyro  X rad/s
        dataFile.print(gyro->gyro.x, 4); dataFile.print(",");
        // Gyro Y rad/s
        dataFile.print(gyro->gyro.y, 4); dataFile.print(",");
        // Gyro Z rad/s
        dataFile.println(gyro->gyro.z, 4);

        dataFile.flush();
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
static void fire_auto_transition_to(door_states_id_t s_id, cck_time_t t)
{
    door_state_t *ds = door_get_state(s_id);
    if(!ds) return;
    door_auto_evt_ctx_t d_evt = {
        .next_state = &ds->base_state
    };
    door_fire_event(DOOR_AUTO_TRANSITION, t, &d_evt);
}
static void blink_pixel(uint16_t hue, uint8_t sat, cck_time_t elapTime)
{
    door_sm.cfg.builtInNeo->setPixelColor(
        0,
        door_sm.cfg.builtInNeo->gamma32(
            door_sm.cfg.builtInNeo->ColorHSV(hue, sat, Adafruit_NeoPixel::sine8((uint8_t)(elapTime>>2))>>2)
        )
    );
    door_sm.cfg.builtInNeo->show();
}
static void display_error(const char* errorMsg)
{
    door_sm.cfg.display->clearDisplay();
    door_sm.cfg.display->setTextSize(2);
    door_sm.cfg.display->setTextColor(COLOR565(255,0,0));
    display_center(errorMsg);
    door_sm.cfg.display->display();
}
static void display_center(const char *str)
{
    int16_t x1, y1;
    uint16_t w, h;
    door_sm.cfg.display->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    door_sm.cfg.display->setCursor(door_sm.cfg.display->width()/2 - w/2,door_sm.cfg.display->height()/2 - h/2);
    door_sm.cfg.display->print(str);
}
static void display_bot_center(const char *str)
{
    int16_t x1, y1;
    uint16_t w, h;
    door_sm.cfg.display->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    door_sm.cfg.display->setCursor(door_sm.cfg.display->width()/2 - w/2, door_sm.cfg.display->height()-h);
    door_sm.cfg.display->print(str);
}
static void display_top_center(const char *str)
{
    int16_t x1, y1;
    uint16_t w, h;
    door_sm.cfg.display->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    door_sm.cfg.display->setCursor(door_sm.cfg.display->width()/2 - w/2, 0);
    door_sm.cfg.display->print(str);
}
static void display_default_settings()
{
    door_sm.cfg.display->clearDisplay();
    door_sm.cfg.display->setTextColor(SH110X_WHITE);
    door_sm.cfg.display->setTextSize(1);
    door_sm.cfg.display->setCursor(0,0);
}
static void time_bar(float scale_factor)
{
    door_sm.cfg.display->drawLine(
        door_sm.cfg.display->width()-1, door_sm.cfg.display->height()-1,
        door_sm.cfg.display->width()-1, (door_sm.cfg.display->height()-1)*scale_factor,
        SH110X_WHITE
    );
}
static void halt_with_reason(const char reason[])
{
    Serial.println(reason);
    while(1);
}

static void halt()
{
    while(1);
}
//===================================================================