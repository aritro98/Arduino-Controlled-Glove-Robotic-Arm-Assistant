# Arduino Controlled Glove Robotic Arm Assistant

A six-axis robotic arm controlled by hand gestures using an Arduino-based sensor glove and ESP‑NOW wireless protocol. Designed to empower individuals with limited mobility by enabling one‑arm operation to manipulate objects with precision.

## About the Project
This project integrates a flex‑sensor glove with an Arduino microcontroller to command a 5‑degree‑of‑freedom robotic arm. The glove captures finger movements and translates them to servo positions, enabling intuitive human‑machine interaction.

**Key goals**:
- Real‑time motion mapping
- Modular firmware for easy extension
- 3D‑printed and CAD‑designed structural parts

## Introduction
Many assistive robotic systems require complex multi‑limb coordination or pre‑programmed routines. Our project addresses the challenge faced by users who can only operate one arm by enabling real‑time manual control of a robotic arm via a wearable sensor glove. By mapping hand orientation (pitch, roll, yaw) to servo movements, users can intuitively manipulate the arm in six degrees of freedom.

## Project Overview
Our system consists of two main components:
* **Robotic Glove**: A wearable glove embedded with flex sensors and an Arduino microcontroller. It captures finger and hand movements and translates them into control signals.
* **Robotic Arm**: A six-degree-of-freedom arm driven by servo motors, also interfaced with an Arduino board. It receives signals from the glove and mirrors the wearer’s gestures.

This setup achieves real-time mapping of human hand gestures to robotic motion, allowing fine manipulation of the arm with a single wearable device. The design files for the mechanical arm are provided in Fusion 360 format (`.f3d`), and the control logic is implemented in Arduino sketches.