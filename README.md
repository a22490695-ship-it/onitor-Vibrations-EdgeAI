Intelligent Structural Integrity Monitor (Edge AI) 🛠️🤖

Predictive Maintenance System based on TinyML and ESP32

📖 Project Overview

This project is a cyber-physical system designed to detect and classify mechanical faults in DC motors (such as unbalance, obstruction, or critical breakage) using Edge AI.

Powered by an ESP32 microcontroller and an MPU6050 accelerometer, the system analyzes vibration spectral signatures in real-time. It features a professional Python Dashboard for remote monitoring and implements an autonomous Active Safety Protocol, executing emergency stops within 200ms upon detecting critical failures.

🎥 WATCH PROJECT DEMO VIDEO

🚀 Key Features

Edge Computing: Real-time inference (10ms latency) running fully offline on the ESP32.

High Precision: Neural Network model trained with Edge Impulse achieving 99.8% accuracy.

Active Safety: Automatic motor emergency stop mechanism if "ROTO" (Broken) state > 85%.

HMI Dashboard: Modern Dark-UI Python application for real-time telemetry visualization.

Robustness: Implements I2C anti-freeze watchdog and auto-recovery logic.

📂 Repository Structure

File

Description

Sensor_Vibraciones_Con_ReintentodeEncendidoFINAL.ino

Master Firmware: Includes AI inference, safety logic, and telemetry.

Sensor_Vibraciones_ConexionEdgeImpulseFINAL.ino

Data Ingestion: Optimized firmware for raw data collection.

tablero.py

HMI Application: Main script for the visualization dashboard.

VIBRATIONS_SENOR_MOTOR.pdf

Technical Report: Full engineering documentation.

requirements.txt

Dependencies required to run the interface.

🛠️ Hardware Setup

The system is built upon the following components:

MCU: ESP32-WROOM-32 DevKit V1.

Sensor: MPU6050 (Accelerometer/Gyroscope).

Driver: L298N H-Bridge.

Actuators: DC Motors with Gearbox.

(Refer to the PDF Report for the complete schematic diagram)

💻 Installation & Usage

1. Firmware Flashing

Install the Edge Impulse Library (exported as .zip) in your Arduino IDE.

Open the Master Firmware (.ino).

Select Board: DOIT ESP32 DEVKIT V1.

Upload the code to the microcontroller.

2. Running the Dashboard

Ensure you have Python installed.

Install dependencies:

pip install -r requirements.txt


Run the application:

python tablero.py


Connect the ESP32 via USB and select the COM Port in the interface.

📊 Model Performance

The machine learning model was trained using Spectral Analysis signal processing blocks and a Keras Neural Network.

Training Data: >45 minutes of vibration data at 100Hz.

Classes:

NORMAL: Optimal operation.

ROTO (Broken): Simulated critical failure (mass unbalance).

OBSTRUCCION (Obstruction): Simulated mechanical friction.

NADA (Idle): Motor off.

👥 Authors

Project developed for the Engineering Contest by:

Alcaraz Reyes Yael Estheban

Licea Murillo Alan

Tapia Gonzalez Axel Roberto

November 2025
