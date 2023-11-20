# GRC Development Board Guide

## Description

GRC Dev Board has:

* 2 ultra-compact, low-power, omnidirectional, digital MEMS microphones
* A multi-chip module MPU-9250 (9-axis sensor: a gyroscope, an accelerometer, and a compass)
* 3 RGB LEDs
* 2 buttons: Reset and User/Boot
* OLED Display (optional)

<img src="media/DevBoard_Description.png">

## Power

* Development Board
  * via USB-port
  * from battery
* GRC
  * from Dev Board
  * With another device:
    * via power pins (see Figure 2)
    * via USB-port

## Connection

### Development Board

The Dev Board is delivered with all connections made and ready for work.

|   | **SDA** | **SCL** |
| -- | ------- | ------- |
| GRC | GPIO_0 | GPIO_1 |
| DevBoard |  GPIO_12 | GPIO_13 |

### Other devices

<img src="media/DevBoard_Connection_Scheme.png">

For attaching your own device to GRC, connect SDA, SCL, GND pins correspondingly to GRC.

When connecting SDA and SCL line, it is required to provide outline to power via 4.7 kΩ resistor.

## Demo

GRC_AI_SW_DEMO (Dev Board is delivered with demo firmware pre-installed) aims to showcase work with device peripherals:

* GRC
* Display
* Buttons
* Device memory
* Built-in accelerometer
* LEDs
* Microphone
* Speaker

The demo offers three test scenarios, in which data are received from an accelerometer, pre-processed and sent for training/classification at GRC AI SW over I2C.

**GestureProject** – allows training in several hand gestures (i.e. types of Dev Board movement) for their further recognition and classification. For example, moving the Dev Board Left-Right or in a circle. Currently, the solution incorporates smoothing and dimensionality reduction (average out of 10 measurements) of data received from the accelerometer. It helps to remove slight deviations (e.g., trembling hands).

**VibroProject** – allows recognizing different types of vibrations, for example, different fan work modes. The data received from an accelerometer are unfolded, normalized, and filtered. During the recognition process, the device shall be in the same position as it was during training, otherwise, correct results are not guaranteed.

**RhythmProject** – recognize different tap rhythms. The data outliers are identified and the distances between them are calculated.

## Demo Project Structure

* **App** – description of demo scenarios with user interface: menus, navigation, actions with buttons for each of the test scenarios given above;
  * **include** – provides interfaces for work with peripherals (IButton.hpp, IDisplay.hpp, ILed.hpp. ISensor.hpp, IStorage.hpp);
  * **App.hpp/App.cpp** – provides description of app work: status processing and change, event management, interaction with peripherals;
  * **GestureScenario.cpp** – provides work with App events for the gesture recognition scenario;
  * **RhythmScenario.cpp** – provides work with App events for the rhythm pattern recognition scenario;
  * **VibroScenario.cpp** – provides work with App events for the vibration recognition scenario.
* **common** – mathematical functions for signal processing:
  * **common.hpp/common.cpp** – performs transformation of raw data into an array and allows saving an array into a file.
  * **SignalProcessing.hpp/SignalProcessing.cpp** – provides functions for signal processing (signal as array): normalize, shift, scale, and rotate. Reads statistics on the processed data (mean and standard deviation);
  * **custom_types.h.** – provides structure description for configuring detection parameters;
  * **hyper_parameters.cpp** – provides parameter values for each of the demo scenarios.
* **GRC** – GRC demo scenarios performing data receiving and pre-processing:
  * **IGrc.hpp/IGrc.cpp** – GRC interaction interface using GRC_SDK;
  * **GestureApp.hpp/GestureApp.cpp, VibroApp.hpp/VibroApp.cpp, RhythmApp.hpp/RhythmApp.cpp** – implementation of receiving and saving data, model training, and signal recognition for each of the scenarios described above.
* **hardware** – device peripherals implementation
  * **Accelerometer.hpp/Accelerometer.cpp** – configuring and reading data from accelerometer;
  * **Button.hpp/Button.cpp** – work with built-in buttons;
  * **i2c.h/i2c.cpp** – work with I2C interface;
  * **Lcd.h/Lcd.cpp** – work with OLED-display;
  * **Led.h/Led.cpp** – LEDs control;
  * **NVStrorage.hpp/NVStrorage.cpp** – interaction interface with non-volatile memory;
  * **utils.hpp** – signal to reset GRC.
