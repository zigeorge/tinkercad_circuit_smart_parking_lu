# IoT-Based Smart Parking Space Monitoring System

This repository contains a Tinkercad-based prototype of an IoT smart parking system designed for a university parking lot scenario. The project demonstrates parking occupancy detection, obstacle monitoring, local visual feedback, gate access control, and simulated IoT dashboard output using Arduino-based edge nodes.

## Project Overview

The system is divided into two Arduino Uno modules:

1. **Parking Monitoring Node**
   - Detects parking occupancy for four parking spaces.
   - Detects nearby obstacles from the left, front, and right directions.
   - Activates a buzzer when an obstacle is detected within the safety threshold.
   - Displays the number of available parking spaces on a 16x2 LCD.
   - Uses a NeoPixel strip to show parking availability and gate activity.
   - Prints parking and obstacle data to the Serial Monitor as simulated IoT dashboard output.

2. **Gate Access Node**
   - Uses a 4x4 keypad for password-based access control.
   - Controls entrance and exit gates using two servo motors.
   - Uses a PIR sensor near the gate area to detect vehicle movement after the system is unlocked.
   - Sends a gate-open signal to the Parking Monitoring Node.
   - Prints access events to the Serial Monitor as simulated core-system data.

## Main Features

- Four parking slots monitored using ultrasonic sensors.
- Three-direction obstacle detection: left, front, and right.
- Buzzer warning for nearby obstacles.
- LCD display showing available parking count.
- NeoPixel strip:
  - Green when at least one parking space is available.
  - Red when all parking spaces are occupied.
  - Blinks red, green, blue, and yellow when a gate opens.
- Password-based gate access using keypad.
- Servo-controlled entrance and exit gates.
- PIR-based vehicle movement detection near the gate.
- Serial Monitor output representing data that could be sent to an IoT dashboard.

## Hardware Components

| Component | Quantity | Purpose |
|---|---:|---|
| Arduino Uno R3 | 2 | Parking monitoring node and gate access node |
| Breadboard | 2 | Power and ground distribution |
| 3-pin ultrasonic distance sensor | 7 | Parking slot and obstacle detection |
| PIR sensor | 1 | Vehicle movement detection near gate |
| 4x4 keypad | 1 | Password input |
| Micro servo motor | 2 | Entrance and exit gate control |
| 16x2 LCD | 1 | Available parking display |
| NeoPixel strip | 1 | Parking status and gate activity indicator |
| Piezo buzzer | 1 | Obstacle warning alarm |
| Jumper wires | As needed | Circuit connections |

## Node 1: Parking Monitoring Node

The Parking Monitoring Node is responsible for detecting parking availability and obstacle conditions.

### Pin Configuration

| Component | Arduino Pin |
|---|---:|
| Parking Slot 1 ultrasonic SIG | D2 |
| Parking Slot 2 ultrasonic SIG | D3 |
| Parking Slot 3 ultrasonic SIG | D4 |
| Parking Slot 4 ultrasonic SIG | D5 |
| Left obstacle ultrasonic SIG | D6 |
| Front obstacle ultrasonic SIG | D7 |
| Right obstacle ultrasonic SIG | D8 |
| Buzzer positive | D9 |
| NeoPixel signal input | D10 |
| Gate signal input from Gate Arduino | D11 |
| LCD SDA | A4 |
| LCD SCL | A5 |

### LCD Behavior

When the simulation starts, the LCD displays a welcome message. After startup, it shows only the number of available parking spaces.

Example:

```text
Available:
4
```

### NeoPixel Behavior

| Condition | NeoPixel Output |
|---|---|
| At least one parking slot is available | All LEDs green |
| No parking slot is available | All LEDs red |
| Entrance or exit gate opens | Blinks red, green, blue, and yellow |

### Obstacle Warning

The left, front, and right ultrasonic sensors monitor nearby obstacles. If an object is detected within the safety threshold, the buzzer activates.

## Node 2: Gate Access Node

The Gate Access Node is responsible for password-based access and gate movement.

### Pin Configuration

| Component | Arduino Pin |
|---|---:|
| Keypad R1 | D2 |
| Keypad R2 | D3 |
| Keypad R3 | D4 |
| Keypad R4 | D5 |
| Keypad C1 | D6 |
| Keypad C2 | D7 |
| Keypad C3 | D8 |
| Keypad C4 | D9 |
| Entrance servo signal | D10 |
| Exit servo signal | D11 |
| Gate-open signal to Parking Arduino | D12 |
| PIR sensor OUT | A0 |

### Keypad Logic

The fixed password is:

```text
1234
```

Commands:

| Input | Meaning |
|---|---|
| `A1234#` | Unlock entrance gate |
| `B1234#` | Unlock exit gate |
| `A9999#` | Invalid entrance password |
| `B9999#` | Invalid exit password |
| `*` | Clear input |

The password does not directly open the gate. A correct password creates an unlocked state. During that unlocked state, PIR motion near the gate triggers the selected servo gate.

## Inter-Arduino Communication

| From | To | Purpose |
|---|---|---|
| Gate Arduino D12 | Parking Arduino D11 | Sends gate-open activity signal |
| Gate Arduino GND | Parking Arduino GND | Common ground reference |

When the gate opens, the Gate Access Node sends a HIGH signal to the Parking Monitoring Node. The Parking Monitoring Node then runs the NeoPixel gate animation.

## System Logic

### Parking Monitoring Logic

1. Read four parking slot ultrasonic sensors.
2. Count occupied and available spaces.
3. Display available parking count on the LCD.
4. Show green NeoPixel when parking is available.
5. Show red NeoPixel when parking is full.
6. Read left, front, and right obstacle sensors.
7. Activate buzzer if an obstacle is within the threshold.
8. Print parking and obstacle status to the Serial Monitor.

### Gate Access Logic

1. User enters a keypad command.
2. `A` selects entrance gate and `B` selects exit gate.
3. User enters password `1234` and presses `#`.
4. If the password is correct, the selected gate is unlocked.
5. PIR motion near the gate triggers the selected servo.
6. Gate activity signal is sent to the Parking Monitoring Node.
7. NeoPixel strip shows gate activity.
8. Access events are printed to the Serial Monitor.

## Serial Monitor Output

The Parking Monitoring Node prints:

- Slot distance readings.
- Occupied or available status for each parking space.
- Total occupied and available count.
- Obstacle sensor readings.
- Obstacle alert status.

The Gate Access Node prints:

- Keypad input events.
- Password validation result.
- Gate unlock events.
- Gate open and close events.
- Simulated core-system access payload.

## Simulated IoT Dashboard Concept

The Tinkercad prototype does not connect to a real network. Instead, the Serial Monitor output represents the type of data that could be sent to a real IoT dashboard or university parking management system.

In a real implementation, each parking node could send data using Wi-Fi, MQTT, HTTP, or another IoT communication protocol. A central system could display:

- Total available spaces.
- Zone-wise parking availability.
- Occupancy percentage.
- Vehicle entry and exit activity.
- Obstacle or safety alerts.

## Current Prototype Scope

The current implementation demonstrates one parking section with four parking spaces. The modular design can be extended to multiple parking zones by replicating the parking monitoring node.

## Limitations

- The circuit is simulated in Tinkercad.
- Serial Monitor output is used to simulate IoT dashboard data.
- PIR sensors detect motion rather than fixed vehicle presence or distance.
- In a real gate safety system, an ultrasonic sensor, infrared beam sensor, or pressure loop would be safer for detecting a stopped vehicle under a gate.
- The access control uses one fixed password. A real deployment would require stronger authentication and secure credential management.

## Future Enhancements

- Add Wi-Fi-enabled boards such as ESP32.
- Send data to a cloud dashboard using MQTT or HTTP.
- Add a central dashboard for live parking analytics.
- Add multiple parking zones.
- Add dedicated disabled parking detection.
- Replace PIR gate sensing with a distance-based or beam-based safety sensor.
- Add license plate recognition or university parking permit integration.
- Add mobile app support for live parking availability.
- Add solar power and weatherproof hardware for real outdoor deployment.

## Project Status

The current Tinkercad circuit demonstrates:

- Parking occupancy monitoring.
- Obstacle warning.
- LCD availability display.
- NeoPixel visual indicator.
- Password-based gate access.
- Servo gate operation.
- PIR-based gate movement detection.
- Simulated IoT dashboard output through Serial Monitor.
