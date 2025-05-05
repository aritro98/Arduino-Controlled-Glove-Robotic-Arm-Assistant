# Arduino Controlled Glove Robotic Arm Assistant

A six-axis robotic arm controlled by hand gestures using an Arduino-based sensor glove and ESP‑NOW wireless protocol. Designed to empower individuals with limited mobility by enabling one‑arm operation to manipulate objects with precision.

## About the Project
This project integrates a flex‑sensor glove with an Arduino microcontroller to command a 5‑degree‑of‑freedom robotic arm. The glove captures finger movements and translates them to servo positions, enabling intuitive human‑machine interaction.

**Key goals**:
- **Healthcare & Medical Assistance**: Precision in minimally invasive procedures.
- **STEM Education**: Hands‑on robotics learning.
- **Household Automation**: Aid elderly or disabled in daily tasks.

## Introduction
Many assistive robotic systems require complex multi‑limb coordination or pre‑programmed routines. Our project addresses the challenge faced by users who can only operate one arm by enabling real‑time manual control of a robotic arm via a wearable sensor glove. By mapping hand orientation (pitch, roll, yaw) to servo movements, users can intuitively manipulate the arm in six degrees of freedom.

## Overview
Our system consists of two main components:
* **Robotic Glove**: A wearable glove embedded with flex sensors and an Arduino microcontroller. It captures finger and hand movements and translates them into control signals.
* **Robotic Arm**: A six-degree-of-freedom arm driven by servo motors, also interfaced with an Arduino board. It receives signals from the glove and mirrors the wearer’s gestures.

This setup achieves real-time mapping of human hand gestures to robotic motion, allowing fine manipulation of the arm with a single wearable device. The design files for the mechanical arm are provided in Fusion 360 format (`.f3d`), and the control logic is implemented in Arduino sketches.

## Features
- **Glove Sensors**: Five flex sensors for individual finger tracking
- **Arduino Control**: Arduino sketches (`Robotic_Glove.ino`, `Robotic_Arm.ino`)
- **CAD Designs**: Fusion 360 `.f3d` files for both glove mount and arm segments
- **Power Management**: Efficient servo power distribution
- **Modular Code**: Separate modules for sensor reading and servo actuation

## Workflow
1. **Sensor Calibration**:
   * Calibrate flex sensors on the glove to establish bending thresholds (e.g. neutral, half‑bent, fully bent).
   * Zero the MPU6050 (accelerometer/gyro) so that pitch, roll, and yaw read 0° when the hand is in the reference pose.
2. **Gesture Acquisition**:
   * User dons the ESP32‑based glove.
   * MPU6050 continuously measures orientation (pitch, roll, yaw).
   * Arduino ADC reads analog voltages from each flex sensor.
3. **Raw Data Conversion & Mapping**:
   * Convert MPU6050 outputs from radians to degrees.
   * Map orientation values (e.g. –90…+90°) into the servo’s 0–180° range.
   * Map each flex‑sensor reading (e.g. 0–1023 ADC) into a corresponding servo angle using the calibrated thresholds.
4. **Mode Selection**:
   * A hardware "MODE" button on the glove toggles between mapping schemes (e.g. orientation‑only, flex‑only, combined control).
   * Debounce the button and update an internal mode flag.
5. **Command Formatting & Wireless Transmission**:
   * For the current mode, package the six target angles into a comma‑delimited ASCII string:
   ```matlab
   angle₁,angle₂,angle₃,angle₄,angle₅,angle₆
   ```
   * Send the string via ESP‑NOW (or Bluetooth/serial) from the glove’s ESP32 to the arm’s ESP32.
6. **Reception & Actuation**:
   * The robotic‑arm ESP32 listens for incoming ESP‑NOW packets (or Bluetooth serial frames).
   * Upon receipt, parse the comma‑delimited string into six integer angles.
   * Write each angle to its corresponding servo driver, moving the joints to replicate the glove posture.
7. **Feedback Display & Loop**:
   * On the glove's OLED, show the current mode and real‑time servo‑angle values.
   * On the arm or a PC GUI, display live status or use LEDs to confirm successful command execution.
   * Loop back to Step 2 continuously for real‑time control.

## Hardware Requirements
- Arduino Uno and Nano (x1 each)
- Flex Sensor (10kΩ) (x5)
- ESP32 Dev Kits (×2)
- MPU6050 IMU module
- 6 × SG90 or similar micro servos
- SSD1306 OLED display (optional)
- Jumper wires, breadboard, and power supply

## Software Requirements
- Arduino IDE (v1.8.x or higher)
- Fusion 360 (for `.f3d` files)
- Python 3.x (optional, for data logging scripts)
- ESP32 board support installed
- `ESP32Servo`, `MPU6050`, `esp_now`, `Adafruit_SSD1306` libraries

## Installation & Setup
1. Clone the repository:
   ```
   git clone https://github.com/aritro98/Arduino-Controlled-Glove-Robotic-Arm-Assistant.git
   cd Arduino-Controlled-Glove-Robotic-Arm-Assistant
   ```
2. Hardware Assembly:
   - 3D print or assemble parts based on the provided Fusion360 (`.f3d`) files.
   - Assemble servo motors, linkages, and mount on the base.
   - Connect flex sensors to the glove's Arduino (analog pins A0–A4).
   - Follow the schematic in `hardware/schematics/circuit_diagram.png`.
3. Open Firmware:
   - Launch Arduino IDE and open `firmware/Robotic_Glove.ino` and `firmware/Robotic_Arm.ino` respectively.
   - Select Arduino Uno board and COM port and upload.