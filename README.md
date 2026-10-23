DeepSync — Sleep Apnea Monitoring Prototype

An Arduino-based university engineering prototype designed to monitor breathing-related movement, heart rate and estimated blood oxygen saturation (SpO₂), with visual status indication and Bluetooth serial output.

> **Project status:** Original prototype code and hardware photograph uploaded. Full compilation, hardware verification and performance testing are pending. This is an educational prototype, **not a medical device** and not intended to diagnose sleep apnea or provide safety-critical alerts.

Prototype hardware

DeepSync assembled hardware prototype

Arduino Nano-based prototype assembled on stripboard. The image shows the physical build; individual modules and wiring require verification against the final circuit.

Intended functionality

The Arduino sketch implements:

• Reading an analogue stretch sensor to estimate breathing-related movement.
• Five-second startup calibration of the breathing signal.
• Signal smoothing and hysteresis to reduce sensitivity to small fluctuations.
• A 10-second threshold for lack of detected breathing movement.
• Reading heart rate and estimated SpO₂ using a MAX30102 sensor interface.
• Five-sample rolling averages for heart rate and SpO₂.
• LED indication of sensor/monitoring states.
• Status messages over USB serial and an HC-05-compatible Bluetooth serial connection.

These are features present in the source code, not yet verified performance claims.

Hardware and connections described in the sketch

|Component              |Connection / role                            |
|-----------------------|---------------------------------------------|
|Arduino Nano           |Main microcontroller                         |
|Analogue stretch sensor|`A0` — breathing movement input              |
|MAX30102 optical sensor|I²C, address `0x57` (`SDA`/`SCL` on the Nano)|
|LED                    |Digital pin `D7` — visual status             |
|Bluetooth serial module|SoftwareSerial `RX=D3`, `TX=D6`              |

The precise power circuitry, signal-level compatibility and final wiring should be checked before powering or testing the device.

LED behaviour programmed in the sketch

|LED state |Meaning in code                                        |
|----------|-------------------------------------------------------|
|Steady on |Stable pulse detected; no breathing pause flagged      |
|Slow blink|No breathing movement detected for more than 10 seconds|
|Fast blink|Breathing pause flagged and estimated SpO₂ below 95%   |
|Off       |No stable pulse/finger detection                       |

Note: The code also reports low SpO₂ by serial/Bluetooth when breathing movement is detected, but does not currently give that condition a separate LED pattern.

Source code

The Arduino sketch is at DeepSync/DeepSync.ino.

It uses the Arduino Wire and SoftwareSerial libraries, plus the third-party DFRobot_BloodOxygen_S library. The exact board, library version and sensor compatibility will be confirmed during testing.

Engineering concepts

Embedded C++, analogue sensor acquisition, I²C communication, serial/Bluetooth communication, calibration, filtering, hysteresis, threshold-based detection, stripboard prototyping and system integration.

Planned verification

1. Confirm that the sketch compiles with the intended Arduino board and library versions.
2. Verify wiring, sensor supply voltage and logic levels before powering hardware.
3. Test stretch-sensor calibration and breathing-state detection.
4. Check pulse/SpO₂ data validity and behaviour when no finger is present.
5. Verify LED patterns and Bluetooth output.
6. Document limitations, results and any subsequent code changes.

Limitations and safety

A stretch sensor cannot by itself confirm airflow cessation, and a prototype optical sensor cannot establish a clinical diagnosis. The thresholds, signal quality and finger-detection logic have not been validated. This project is for engineering learning and demonstration only.
My Contribution

DeepSync was developed as part of a university group engineering project. My primary responsibility was the electronics and hardware development of the prototype.

My contributions included:

* Circuit design: Designing the electronic circuit and planning the connections between the Arduino Nano, sensors, Bluetooth module and LED indicator.
* Hardware assembly: Building the physical circuit and integrating the electronic components.
* Prototype testing: Testing the assembled circuit and checking the stretch sensor’s response to manual stretching and release.
* Programming support: Contributing to the development of the Arduino code and its integration with the hardware.

Testing and Limitations

Initial testing focused on verifying the prototype’s electronic functionality and sensor response.

The stretch sensor was tested through manual extension and release rather than being worn during actual breathing measurements. Consequently, reliable respiratory monitoring and sleep apnoea detection were not clinically validated.

This project provided practical experience in circuit design, electronic prototyping, sensor integration and collaborative engineering development.