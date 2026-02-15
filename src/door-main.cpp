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

#include "buttons.h"
#include "cck_types.h"
#include "states.h"
#include "doorStateMachine.h"


#define SERIAL_BAUD_RATE 115200
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define COLOR565(r, g, b) \
    ((uint16_t)(((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define NUMPIXELS         1

#define I2C_LIS3MDL_ADDRESS 0x1E
#define I2C_LSM6DS_ADDRESS  0x6A // D0 Pull high for 0x6B
#define I2C_SH110X_ADDRESS  0x3C // Address 0x3C default

//==== Pin out ====
//#define CLEAR_BTN_PIN       5
#define VBAT_PIN            (A6)  // Pin for reading battery voltage
#define BUILT_IN_PIXEL_PIN  8
#define BUTTON_A            5 //PA15 //CS
#define BUTTON_B            6 //PC7
#define BUTTON_C            9 //PC5


static Adafruit_LSM6DSOX lsm6ds;
static Adafruit_LIS3MDL lis3mdl;
static button_handle_t btnA, btnB, btnC;
static Adafruit_NeoPixel builtInNeo(1, BUILT_IN_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

static void setup_accel_and_mag();
static void setup_sdcard();
static void setup_buttons();
static void setup_gpio();
static void setup_neopixels();
static void setup_state_machine();
static void setup_display();

static void buttonAPress(cck_time_t);
static void buttonBPress(cck_time_t);
static void buttonCPress(cck_time_t);
static void buttonUp(cck_time_t);
static void buttonDown(cck_time_t);
static void toggleLED(cck_time_t);
static void print_file_list();
static void shutdown_sdcard();
static void halt(const char reason[]);
static void halt();


static void buttonUp(cck_time_t startTime){}
static void buttonDown(cck_time_t startTime){}
static void buttonAPress(cck_time_t startTime)
{
    toggleLED(startTime);
    door_fire_event(DOOR_EVENT_BUTTON_1_PRESS, startTime);
}
static void buttonBPress(cck_time_t startTime)
{
    toggleLED(startTime);
    door_fire_event(DOOR_EVENT_BUTTON_2_PRESS, startTime);
}
static void buttonCPress(cck_time_t startTime)
{
    toggleLED(startTime);
    door_fire_event(DOOR_EVENT_BUTTON_3_PRESS, startTime);
}

static void setup_buttons()
{
    btn_initButton(&btnA, BUTTON_A, INPUT_PULLUP, buttonDown, buttonUp, buttonAPress);
    if(!btn_addButton(&btnA)) Serial.println("Could not add BUTTON_A");
    btn_initButton(&btnB, BUTTON_B, INPUT_PULLUP, buttonDown, buttonUp, buttonBPress);
    if(!btn_addButton(&btnB)) Serial.println("Could not add BUTTON_B");
    btn_initButton(&btnC, BUTTON_C, INPUT_PULLUP, buttonDown, buttonUp, buttonCPress);
    if(!btn_addButton(&btnC)) Serial.println("Could not add BUTTON_C");
}

static void setup_gpio()
{
    pinMode(LED_BUILTIN, OUTPUT);
    setup_buttons();
}

static void setup_neopixels()
{
  builtInNeo.begin();
  builtInNeo.show();  // Initialize to 'off'
}

static void setup_sdcard()
{
    if(!SD.begin(SD_DETECT_NONE)) {
        halt("SD Initialization failed.");
    }
    print_file_list();
}

static void shutdown_sdcard()
{
    SD.end();
}

static void setup_display()
{
  Serial.println("128x64 OLED FeatherWing test");
  delay(250); // wait for the OLED to power up
  display.begin(I2C_SH110X_ADDRESS, true);

  Serial.println("OLED begin");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen internally, this will display the splashscreen.
  //display.display();
  //delay(500);

  // Clear the buffer.
  display.clearDisplay();

  display.setRotation(1);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  const char *str = "TIGGER!";
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(display.width()/2 - w/2,display.height()/2 - h/2);
  display.println(str);
  display.display(); // actually display all of the above
  delay(1000);
}

static void setup_accel_and_mag()
{
  Serial.println("Adafruit LSM6DS+LIS3MDL test!");
  bool lsm6ds_success, lis3mdl_success;

  lsm6ds_success = lsm6ds.begin_I2C(I2C_LSM6DS_ADDRESS);
  lis3mdl_success = lis3mdl.begin_I2C(I2C_LIS3MDL_ADDRESS);

  if (!lsm6ds_success){
    Serial.println("Failed to find LSM6DS chip");
    display_error("Failed to find LSM6DS chip");
  }
  if (!lis3mdl_success){
    Serial.println("Failed to find LIS3MDL chip");
    display_error("Failed to find LIS3MDL chip");
  }
  if (!(lsm6ds_success && lis3mdl_success)) {
    while (1) {
      delay(10);
    }
  }

  Serial.println("LSM6DS and LIS3MDL Found!");

  // lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (lsm6ds.getAccelRange()) {
  case LSM6DS_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case LSM6DS_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case LSM6DS_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case LSM6DS_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  // lsm6ds.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  Serial.print("Accelerometer data rate set to: ");
  switch (lsm6ds.getAccelDataRate()) {
  case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  // lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS );
  Serial.print("Gyro range set to: ");
  switch (lsm6ds.getGyroRange()) {
  case LSM6DS_GYRO_RANGE_125_DPS:
    Serial.println("125 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  case ISM330DHCX_GYRO_RANGE_4000_DPS:
    Serial.println("4000 degrees/s");
    break;
  }
  // lsm6ds.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
  Serial.print("Gyro data rate set to: ");
  switch (lsm6ds.getGyroDataRate()) {
  case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  lis3mdl.setDataRate(LIS3MDL_DATARATE_155_HZ);
  // You can check the datarate by looking at the frequency of the DRDY pin
  Serial.print("Magnetometer data rate set to: ");
  switch (lis3mdl.getDataRate()) {
    case LIS3MDL_DATARATE_0_625_HZ: Serial.println("0.625 Hz"); break;
    case LIS3MDL_DATARATE_1_25_HZ: Serial.println("1.25 Hz"); break;
    case LIS3MDL_DATARATE_2_5_HZ: Serial.println("2.5 Hz"); break;
    case LIS3MDL_DATARATE_5_HZ: Serial.println("5 Hz"); break;
    case LIS3MDL_DATARATE_10_HZ: Serial.println("10 Hz"); break;
    case LIS3MDL_DATARATE_20_HZ: Serial.println("20 Hz"); break;
    case LIS3MDL_DATARATE_40_HZ: Serial.println("40 Hz"); break;
    case LIS3MDL_DATARATE_80_HZ: Serial.println("80 Hz"); break;
    case LIS3MDL_DATARATE_155_HZ: Serial.println("155 Hz"); break;
    case LIS3MDL_DATARATE_300_HZ: Serial.println("300 Hz"); break;
    case LIS3MDL_DATARATE_560_HZ: Serial.println("560 Hz"); break;
    case LIS3MDL_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
  }

  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
  Serial.print("Range set to: ");
  switch (lis3mdl.getRange()) {
    case LIS3MDL_RANGE_4_GAUSS: Serial.println("+-4 gauss"); break;
    case LIS3MDL_RANGE_8_GAUSS: Serial.println("+-8 gauss"); break;
    case LIS3MDL_RANGE_12_GAUSS: Serial.println("+-12 gauss"); break;
    case LIS3MDL_RANGE_16_GAUSS: Serial.println("+-16 gauss"); break;
  }

  lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
  Serial.print("Magnetometer performance mode set to: ");
  switch (lis3mdl.getPerformanceMode()) {
    case LIS3MDL_LOWPOWERMODE: Serial.println("Low"); break;
    case LIS3MDL_MEDIUMMODE: Serial.println("Medium"); break;
    case LIS3MDL_HIGHMODE: Serial.println("High"); break;
    case LIS3MDL_ULTRAHIGHMODE: Serial.println("Ultra-High"); break;
  }

  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
  Serial.print("Magnetometer operation mode set to: ");
  // Single shot mode will complete conversion and go into power down
  switch (lis3mdl.getOperationMode()) {
    case LIS3MDL_CONTINUOUSMODE: Serial.println("Continuous"); break;
    case LIS3MDL_SINGLEMODE: Serial.println("Single mode"); break;
    case LIS3MDL_POWERDOWNMODE: Serial.println("Power-down"); break;
  }

  lis3mdl.setIntThreshold(500);
  lis3mdl.configInterrupt(false, false, true, // enable z axis
                          true, // polarity
                          false, // don't latch
                          true); // enabled!
}

static void setup_state_machine() {
    door_sm_cfg_t door_sm_config = {
        .lsm6ds     = &lsm6ds,
        .lis3mdl    = &lis3mdl,
        .dataFile   = NULL,
        .builtInNeo = &builtInNeo,
        .display    = &display
    };
    if(!door_init_state_machine(door_sm_config)) {
        Serial.println("Error failed to setup door state machine");
        display_error("Error failed to setup door state machine");
        return;
    }
}

//==========================================================================
// Main setup and loop
//==========================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    //TODO: Remove, only used for debugging.
    while (!Serial) delay(10);

    setup_gpio();
    setup_neopixels();
    setup_sdcard();
    setup_display();
    setup_accel_and_mag();
    setup_state_machine();
}

void loop(void)
{
    static sensors_event_t accel, gyro, mag, temp;
    static door_sensor_evt_ctx_t sensor_ctx_ptr = {
      .accel = &accel,
      .gyro = &gyro,
      .mag = &mag,
      .temp = &temp
    };

    lsm6ds.getEvent(&accel, &gyro, &temp);
    lis3mdl.getEvent(&mag);
    door_fire_event(
        DOOR_SENSOR_READING,
        millis(),
        &sensor_ctx_ptr
    );
    btn_processButtons();
    door_run_state_machine(millis());

    delay(1);
}
//==========================================================================

//==========================================================================
// Utils
//==========================================================================
static void print_file_list()
{
  File root = SD.openRoot();
  root.ls(LS_R | LS_DATE | LS_SIZE);
  root.close();
}

static void halt(const char reason[])
{
    Serial.println(reason);
    while(1);
}

static void halt()
{
    while(1);
}

static void toggleLED(cck_time_t _)
{
    static bool t = LOW;
    t = !t;
    digitalWrite(LED_BUILTIN, t);
}

static void display_error(const char* errorMsg)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(COLOR565(255,0,0));
    display.getTextBounds(errorMsg, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width()/2 - w/2, display.height()/2 - h/2);
    display.println(errorMsg);
    display.display();
}
//==========================================================================
