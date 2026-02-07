#include <Arduino.h>
#include <STM32SD.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_NeoPixel.h>
#include <stdbool.h>
#include "buttons.h"
#include "cck_types.h"
#include "states.h"
#include "doorStateMachine.h"


#define SERIAL_BAUD_RATE 115200
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define WRITE_DELAY 1000

#define NUMPIXELS         1

//==== Pin out ====
#define CLEAR_BTN_PIN       5
#define VBAT_PIN            (A6)  // Pin for reading battery voltage
#define BUILT_IN_PIXEL_PIN  8

#define FILE_NAME "data.csv"


static Adafruit_LSM6DSOX lsm6ds;
static Adafruit_LIS3MDL lis3mdl;
static File dataFile;
static button_handle_t onlyButton;
static Adafruit_NeoPixel builtInNeo(1, BUILT_IN_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

static void buttonPress(cck_time_t);
static void buttonUp(cck_time_t);
static void buttonDown(cck_time_t);
static void toggleLED(cck_time_t);
static void setup_accel_and_mag();
static void halt(const char reason[]);
static void halt();
static void print_file_list();
static void setup_sdcard();
static void shutdown_sdcard();
static void setup_buttons();
static void setup_gpio();
static void setup_neopixels();
static void print_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp);
static void write_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp);
static void setup_state_machine();


static void setup_neopixels()
{
  builtInNeo.begin();
  builtInNeo.show();  // Initialize to 'off'
}

static void setup_accel_and_mag()
{
  Serial.println("Adafruit LSM6DS+LIS3MDL test!");
  bool lsm6ds_success, lis3mdl_success;

  lsm6ds_success = lsm6ds.begin_I2C();
  lis3mdl_success = lis3mdl.begin_I2C(0x1E);

  if (!lsm6ds_success){
    Serial.println("Failed to find LSM6DS chip");
  }
  if (!lis3mdl_success){
    Serial.println("Failed to find LIS3MDL chip");
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

static void halt(const char reason[])
{
    Serial.println(reason);
    while(1);
}

static void halt()
{
    while(1);
}

static void print_file_list()
{
  File root = SD.openRoot();
  root.ls(LS_R | LS_DATE | LS_SIZE);
  root.close();
}

static void setup_sdcard()
{
    if(!SD.begin(SD_DETECT_NONE)) {
        halt("SD Initialization failed.");
    }

    print_file_list();

    dataFile = SD.open(FILE_NAME, FILE_WRITE);
    if (dataFile) {
        dataFile.seek(dataFile.size()); //Move to the end of the file for appending.
    } else {
        Serial.print("Error opening file "); Serial.println(FILE_NAME);
        halt();
    }
}

static void shutdown_sdcard()
{
    SD.end();
}

static void buttonUp(cck_time_t startTime){}
static void buttonDown(cck_time_t startTime){}
static void buttonPress(cck_time_t startTime)
{
    state_fire_event(&door_state_machine, DOOR_EVENT_BUTTON_1_PRESS, startTime);
}

static void setup_buttons() {
    btn_initButton(&onlyButton, CLEAR_BTN_PIN, INPUT_PULLUP, buttonDown, buttonUp, buttonPress);
    btn_addButton(&onlyButton);
}

static void setup_gpio()
{
    pinMode(CLEAR_BTN_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    setup_buttons();
}

static void print_sensor_data(const sensors_event_t *accel, const sensors_event_t *gyro, const sensors_event_t *mag, const sensors_event_t *temp)
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
        dataFile.print(gyro->gyro.z, 4);

        dataFile.flush();
    } else {
        Serial.print("error on file handle");
    }
}

static void toggleLED(cck_time_t _)
{
    static bool t = LOW;
    t = !t;
    digitalWrite(LED_BUILTIN, t);
}

static void setup_state_machine() {
    idle_event_handlers[DOOR_EVENT_BUTTON_1_PRESS] = toggleLED;
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial) delay(10);

    setup_gpio();
    setup_sdcard();
    setup_accel_and_mag();
    setup_neopixels();
    setup_state_machine();
}

void loop(void)
{
    static sensors_event_t accel, gyro, mag, temp;
    static unsigned long prevTime = 0;

    unsigned long curTime = millis();
    unsigned long elapTime = curTime - prevTime;

    if(elapTime > WRITE_DELAY) {
        prevTime = curTime;
        // Get new normalized sensor events
        lsm6ds.getEvent(&accel, &gyro, &temp);
        lis3mdl.getEvent(&mag);
        print_sensor_data(&accel, &gyro, &mag, &temp);
        write_sensor_data(&accel, &gyro, &mag, &temp);
    }

    btn_processButtons();
    state_machine_run(&door_state_machine, curTime);

    delay(10);
}
