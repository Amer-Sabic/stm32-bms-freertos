# 4S Li-ion Battery Management System (BMS)

A robust, real-time, isolated Battery Management System (BMS) designed for a 4-cell (4S) Lithium-ion battery pack (14.4V–14.8V nominal, 16.8V max). This system provides real-time cell voltage, current, and temperature monitoring, safe load control, wireless communication, and multi-layered hardware/software protection.

---

## 📌 Project Overview

This BMS is built to safely monitor and manage 4S battery packs used in high-performance applications like drones, RC vehicles, and robotics. It features physical galvanic isolation between high-power battery circuitry and user-accessible control logic, run by a dual-microcontroller setup and a prioritized FreeRTOS architecture.

### Key Capabilities
* **Cell Voltage Monitoring:** Individual voltage scaling and multiplexed measurement for each of the 4 cells.
* **Current & Temperature Sensing:** Shunt-based high-side current measurement and NTC thermistor temperature sensing.
* **Galvanic Isolation:** Complete separation of battery ground/power from MCU/USB control to ensure user safety and high noise immunity.
* **State Machine Protection:** Automated fault handling for overvoltage, undervoltage, overcurrent, overtemperature, and measurement timeouts.
* **Wireless Telemetry:** Integrated WiFi module on ESP32 to transmit real-time data to PCs or mobile devices.

---

## 🛠️ Hardware Architecture

### System Core & Components
* **Primary Microcontroller:** **STM32G474RE**
  * Fast ADCs, low-power sleep/standby modes, high-efficiency task handling.
  * Handles data acquisition, state machine execution, and safety protection logic.
* **Secondary Microcontroller:** **ESP32-WROOM-32E**
  * Provides WiFi connectivity for wireless data streaming and remote commands.
* **Power Supply:**
  * **LMR33630 Buck Converter:** Wide input range (3.8V–36V), step-down to 3.3V for internal components with low EMI.
  * **NKE0305SC:** Isolated DC-DC converter providing isolated 3.3V supply across the isolation barrier.
* **Isolation:**
  * **AMC1301DWV:** Isolation amplifiers for signal measurement.
  * **ISO7730 / ISO7710:** Digital isolators for control signals.
* **Analog Circuitry & Load Switch:**
  * **MCP6004:** Operational amplifiers for signal buffering.
  * **TC4427VUA:** Power MOSFET gate driver for battery output control.

---

## 💻 PCB & Layout Highlights

* **Layer Stack:** 4-Layer board (Signal – Ground – Power – Signal), 1.6mm thickness.
* **Isolation Barrier:** A clear physical boundary separating high-power battery ground and microcontroller ground to prevent ground-loop currents and high-frequency noise coupling.
* **Switching Regulator Layout:** Minimized high-frequency current loop paths ($di/dt$) and optimized thermal dissipation via ground vias.
* **Antenna Keep-out:** Dedicated trace/copper keep-out zone beneath the ESP32 PCB antenna.

---

## ⚙️ Software Architecture

The STM32 firmware is built on top of **FreeRTOS** using a finite state machine (FSM) architecture.

### System States
1. **STANDBY:** Initial state. Gate switch is OFF; system waits for user activation via WiFi while confirming system parameters are normal.
2. **ON:** Gate switch is ON; load is powered while continuous measurement verification occurs.
3. **FAULT:** Triggered automatically upon out-of-bounds voltage, current, temperature readings, or measurement timeout. Gate switch instantly shuts OFF.

### FreeRTOS Tasks
* **Acquisition Task:** Reads 4x cell voltages via multiplexers, reads pack temperature via NTC, copies current readings, and pushes data to `measQueue`.
* **Protection Task:** Evaluates received measurements against safety thresholds. Controls the MOSFET driver and handles state transitions.
* **Communication Task:** Serializes telemetry frames with CRC16 validation and communicates with ESP32 via UART/DMA.
* **Supervisor Task:** Watchdog supervisor task (`IWDG`) that ensures all primary tasks regularly flag their liveness bits (`ACQ_BIT`, `PROT_BIT`, `COMM_BIT`). If a task fails, a hardware reset is triggered.

