# LED installations for Burning Man 2018

## Dependencies

- Arduino 1.8.5
- Teensyduino 1.4.2

## Apps

### Disc

#### Driver

- Board: Teensy 3.6
- Dependencies
    - FastLED 3.0.7
    - RF24 1.3.1

#### Remote

- Board: Teensy 3.2
- Dependencies
    - Bounce
    - VL53LOX (Pololu version)
        - Note: you must edit `~/Documents/Arduino/libraries/VL53LOX/VL53L0X.cpp` and replace `#include <Wire.h>` with `#include <i2c_t3.h>`.
    - RF24 1.3.1

### Shoe

- Board: [Pololu A-Star 32U4 Mini SV](https://www.pololu.com/product/3145)
    - Be sure to follow [board setup instructions](https://www.pololu.com/docs/0J61/6.2)
- Dependencies
    - [I2C Master Library](http://dsscircuits.com/index.php/articles/66-arduino-i2c-master-library)
    - [LIS331](https://github.com/szotsaki/LIS331)
    - SimpleKalmanFilter

