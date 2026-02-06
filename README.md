# CCK Door Sensor
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
