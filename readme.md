General Project Information
1. What the project does
This project is a Smart Structural Integrity Monitor System designed to perform predictive maintenance on electric motors. It utilizes an ESP32 microcontroller and vibration sensors (accelerometers) to "listen" to the motor's mechanical health in real-time.

Using Edge Artificial Intelligence (TinyML), the system analyzes vibration patterns and classifies the machine's status into four categories:

NORMAL: Optimal operation.

OBSTRUCTION: Detection of excessive loads or friction.

BROKEN: Detection of critical failures (shaft imbalance).

IDLE: Motor at rest (Nothing).

Additionally, the system features active safety: if it detects a critical failure ("BROKEN"), it automatically cuts power to the motor to prevent catastrophic damage.

2. Why the project is useful
In the industry, unforeseen motor failures cause costly production stoppages ("downtime"). This project is useful because:

Prevents Failures: Detects anomalies before the motor breaks down completely.

Cost-Effective: Uses accessible hardware (ESP32, MPU6050) to democratize technology that is usually very expensive.

Autonomy (Edge AI): By processing data on the chip itself (without relying on the internet/cloud), the response is immediate (<200ms) and secure, making it ideal for industrial environments where latency is critical.

Safety: Automatically protects machinery without human intervention.

3. How users can get started with the project
To replicate or use this system, users should follow these steps:

Hardware: Assemble the circuit following the provided schematic diagram (ESP32 + L298N + MPU6050).

Firmware: Upload the master code (Codigo_Final_Dual_WROOM32.ino) to the ESP32 using the Arduino IDE.

Interface: Install Python and the necessary libraries (pip install customtkinter pyserial).

Execution: Connect the ESP32 to the USB port and run the tablero.py script. The system will start monitoring and automatically display the status on the Dashboard.

4. Where users can get help with your project
Users can find support and detailed documentation at:

GitHub Repository: [Insert your GitHub link here] - To consult source code, diagrams, and updates.

Technical Report: The PDF document attached to the project contains an in-depth engineering explanation, troubleshooting, and connection guides.

Contact: Through the developers' profiles for specific technical inquiries regarding the Edge Impulse implementation.

5. Who maintains and contributes to the project
This project was developed and is maintained by an engineering team consisting of:

Alcaraz Reyes Yael Estheban

Licea Murillo Alan

Tapia Gonzalez Axel Roberto

The development was carried out in the context of Edge AI contest by Edge Impulse, applying Industry 4.0 and Embedded Systems principles.