/*
 * CÓDIGO PARA GRABAR MÁS DATOS (Modo Silencioso)
 * Solo imprime: ax, ay, az
 * Y mantiene los motores encendidos.
 */
#include <Wire.h>
// Configuración Motores
#define MOTOR1_IN1 26
#define MOTOR1_IN2 27
#define MOTOR2_IN3 14
#define MOTOR2_IN4 12
// Configuración Sensor
#define MPU_ADDR 0x68
const float CONVERSION = 9.81 / 4096.0;
void setup() {
  Serial.begin(115200);
  // 1. ENCENDER MOTORES
  pinMode(MOTOR1_IN1, OUTPUT); pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT); pinMode(MOTOR2_IN4, OUTPUT);
  digitalWrite(MOTOR1_IN1, HIGH); digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN3, HIGH); digitalWrite(MOTOR2_IN4, LOW);
  // 2. INICIAR SENSOR (Manual)
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0); Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); Wire.write(0x10); Wire.endTransmission(true);
  delay(100);
}
void loop() {
  // Lectura rápida y limpia
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();
  int16_t AcZ = Wire.read() << 8 | Wire.read();
  // IMPRESIÓN LIMPIA (Solo números para Edge Impulse)
  Serial.print(AcX * CONVERSION); Serial.print(",");
  Serial.print(AcY * CONVERSION); Serial.print(",");
  Serial.println(AcZ * CONVERSION);
  delay(10);
}