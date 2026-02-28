# CCK Door Sensor ([Tigger](https://www.youtube.com/watch?v=dJFyz73MRcg))
The Makefile for this project still depends on the Arduino tool chain and you will need to update the path information if you want to use it.

## Setup
```
make install-deps && make
```

## Flashing
* Requires `dfu-util` be installed.
* Make sure to pull the B0 pin high
* Press the reset button wait a second then
```
make upload
```

## Clean up
```
 make clean-tools clean
```

## How to use the Makefile
```
make help
```

## Dependencies
* https://github.com/adafruit/Adafruit_LIS3MDL
    * https://github.com/adafruit/Adafruit_BusIO
    * https://github.com/adafruit/Adafruit_Sensor
* https://github.com/adafruit/Adafruit_LSM6DS
    * https://github.com/adafruit/Adafruit_BusIO
    * https://github.com/adafruit/Adafruit_Sensor
* https://github.com/adafruit/Adafruit_NeoPixel
* https://github.com/stm32duino/STM32SD
    * https://github.com/stm32duino/FatFs 
* 

```
project/
├── Makefile
├── src/
└── tools/
    ├── arm-gnu-toolchain-14.2.rel1-.../
    ├── Arduino_Core_STM32-2.12.0/
    └── libraries -> ~/Arduino/libraries  (symlink)
```



## Notes
```
dfu-util --alt 0 --dfuse-address 0x08000000:leave --download your_firmware.bin
```
**Flags**
--alt 0 (short form -a):  
Specifies the alternate setting of the DFU interface. For STM32 chips, the internal flash is typically mapped to alternate setting 0. You can verify this by running dfu-util --list.

--dfuse-address 0x08000000:leave (short form -s):  
This is a DfuSe-specific flag used for devices (like STMicroelectronics) that require the host to specify the target memory address.
    0x08000000: The starting memory address for internal flash on the STM32F405.
    :leave: An optional modifier that tells the device to exit DFU mode and start running the new firmware immediately after the flash is finished.

--download your_firmware.bin (short form -D):  
Instructs the tool to write (download from the computer to the device) the specified firmware file. 

Additional Useful Long-Form Flags:  
    --list (-l): Shows all currently connected DFU-capable devices.
    --device VENDOR:PRODUCT (-d): Targets a specific device by its USB IDs (e.g., --device 0483:df11). This is helpful if you have multiple DFU devices connected.
    --reset (-R): Issues a USB reset signal after the operation is complete. 

### Planned state table
```
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│           Pre-Idle                   │
│Animation:                            │
│ * Faded blink pixel warm blue        │
│ * Shows sliding bar count down till  │
│  auto state change                   │
│Events:                               │
│ * auto-trans: after 5s               │
│ * btn-prs: next state idle           │
└──────────────────────────────────────┘
                    |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│               Idle                   │
│Animation:                            │
│ * Pixel color warm blue              │
│ * Prints Idle on screen or anim      │
│ * Prints max Gs from last run        │
│Events:                               │
│ * btn-prs: next state pre-newfile    │
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
                   |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│             Pre-Calibrate            │
│Animation:                            │
│ * Pixel color light pink             │
│ * Shows sliding bar count down till  │
│  auto state change                   │
│Events:                               │
│ * auto-trans: Calibrate after 3s     │
│ * btn-prs: next state Pre-Idle       │
└──────────────────────────────────────┘
                   |
┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┑
│             Calibrating              │
│Animation:                            │
│ * Pixel color flash light pink       │
│ * Shows sliding bar count down till  │
│ * Takes calibration readings         │
│  auto state change                   │
│Events:                               │
│ * btn-prs: next state Pre-Idle       │
└──────────────────────────────────────┘
                    
```
