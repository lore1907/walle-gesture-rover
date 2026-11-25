# WALL-E | Gesture-Controlled Rover

WALL-E is a collaborative robotics project focused on
the development of a gesture-controlled rover based on ESP32.

The system integrates computer vision, embedded programming,
and wireless communication to control a robotic vehicle
through hand gestures captured by a computer webcam.

## Features

- Real-time hand gesture recognition
- Wireless communication via Wi-Fi and UDP
- ESP32-based embedded control
- DC motor control using PWM
- Hardware-software integration
- Modular firmware and controller architecture

## System Architecture

The computer captures hand gestures through a webcam
and processes them using a Python-based controller.

Movement commands are transmitted wirelessly to the
ESP32 via UDP. The microcontroller processes the
received commands and controls the rover's motors.

## Tech Stack

### Software
- Python
- C++
- OpenCV
- MediaPipe
- Arduino Framework

### Hardware
- ESP32
- DRV8833 motor driver
- DC motors
- Robotic chassis

### Communication
- Wi-Fi
- UDP

## Repository Structure

- `firmware/`: ESP32 firmware and motor control
- `controller/`: gesture recognition and wireless control
- `tests/`: individual component testing
- `docs/`: technical documentation

## Project Status

This project is currently under active development.

Planned improvements include:

- Improved motor control and acceleration
- Obstacle detection
- Communication timeout and safety mechanisms
- First-person view (FPV) video streaming
- Improved system reliability

## Contributors

This project is collaboratively developed by
Lorenzo Bartoli and Michele Contino, Computer Engineering students at the University
of Naples Federico II.
