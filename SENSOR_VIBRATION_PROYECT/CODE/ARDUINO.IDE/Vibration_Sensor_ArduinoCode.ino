/*
 * CÓDIGO MAESTRO: RETARDO DE 5 SEGUNDOS PARA VISUALIZAR FALLA
 * Hardware: ESP32 WROOM-32 + 2 Sensores + 2 Motores
 */

#include <Vibraciones_inferencing.h>
#include <Wire.h>

// --- DIRECCIONES DE LOS SENSORES ---
#define MPU1_ADDR 0x68 
#define MPU2_ADDR 0x69 
const float CONVERSION_G = 9.81 / 4096.0;

// --- UMBRALES ---
const float UMBRAL_OFF = 0.06; // Para detectar si está apagado
const float UMBRAL_ROTO = 0.85;

// --- TEMPORIZADORES ---
const unsigned long TIEMPO_VISUALIZACION = 5000; // 5 segundos viendo la falla antes de apagar
const unsigned long CASTIGO = 5000;              // Tiempo que dura apagado después

// --- PINES DE MOTORES ---
#define MOTOR1_IN1 26
#define MOTOR1_IN2 27
#define MOTOR2_IN3 14
#define MOTOR2_IN4 12

// Variables Globales
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
bool mpu1_conectado = false;
bool mpu2_conectado = false;

// Estados de Bloqueo (Protección final)
bool bloqueoMotor1 = false;
bool bloqueoMotor2 = false;
unsigned long tiempoBloqueo1 = 0;
unsigned long tiempoBloqueo2 = 0;

// Estados de "Pre-Falla" (Los 5 segundos de espera)
bool contandoFallaM1 = false;
unsigned long inicioFallaM1 = 0;

bool contandoFallaM2 = false;
unsigned long inicioFallaM2 = 0;

void setup() {
  Serial.begin(115200);

  // Configurar pines
  pinMode(MOTOR1_IN1, OUTPUT); pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT); pinMode(MOTOR2_IN4, OUTPUT);

  // Arrancar motores
  encenderMotor1();
  encenderMotor2();

  // Configuración I2C
  Wire.begin(21, 22);
  Wire.setClock(400000);
  Wire.setTimeOut(50); 

  mpu1_conectado = iniciarSensor(MPU1_ADDR);
  mpu2_conectado = iniciarSensor(MPU2_ADDR);

  Serial.println("SISTEMA LISTO");
  delay(1000);
}

bool iniciarSensor(int addr) {
  Wire.beginTransmission(addr); Wire.write(0x6B); Wire.write(0);
  if (Wire.endTransmission(true) != 0) return false;
  Wire.beginTransmission(addr); Wire.write(0x1C); Wire.write(0x10);
  return (Wire.endTransmission(true) == 0);
}

void loop() {
  // 1. Gestionar recuperación de bloqueos (cuando ya pasaron los 5s de castigo)
  gestionarBloqueos();

  // 2. Gestionar la CUENTA REGRESIVA DE FALLA (Los 5s de visualización)
  gestionarCuentaRegresiva();

  float roto1 = 0, obs1 = 0, nada1 = 0;
  float roto2 = 0, obs2 = 0, nada2 = 0;
  float estadoBloqueo = 0.0;
  long tiempoRestanteMs = 0;

  // ==========================================
  // MOTOR 1
  // ==========================================
  if (mpu1_conectado && !bloqueoMotor1) {
    if (leerVibracion(MPU1_ADDR)) {
      if (motorEstaQuieto()) {
        nada1 = 1.0; 
        contandoFallaM1 = false; // Si se apaga manual, cancelamos la cuenta
      } else {
        analizarVibracion(roto1, obs1);
        
        // SI DETECTAMOS ROTO Y NO ESTAMOS YA CONTANDO
        if (roto1 > UMBRAL_ROTO && !contandoFallaM1) {
          contandoFallaM1 = true;
          inicioFallaM1 = millis(); // Empezamos a contar 5 segundos
          Serial.println("! ALERTA M1: INICIANDO CUENTA 5s !");
        }
      }
    }
  } else if (bloqueoMotor1) {
    roto1 = 0.0; 
    estadoBloqueo = 1.0;
    long restante = CASTIGO - (millis() - tiempoBloqueo1);
    if (restante > tiempoRestanteMs) tiempoRestanteMs = restante;
  }

  // ==========================================
  // MOTOR 2
  // ==========================================
  if (mpu2_conectado && !bloqueoMotor2) {
    if (leerVibracion(MPU2_ADDR)) {
      if (motorEstaQuieto()) {
        nada2 = 1.0; 
        contandoFallaM2 = false;
      } else {
        analizarVibracion(roto2, obs2);
        
        // SI DETECTAMOS ROTO Y NO ESTAMOS YA CONTANDO
        if (roto2 > UMBRAL_ROTO && !contandoFallaM2) {
          contandoFallaM2 = true;
          inicioFallaM2 = millis(); // Empezamos a contar 5 segundos
          Serial.println("! ALERTA M2: INICIANDO CUENTA 5s !");
        }
      }
    }
  } else if (bloqueoMotor2) {
    roto2 = 0.0;
    estadoBloqueo = 1.0;
    long restante = CASTIGO - (millis() - tiempoBloqueo2);
    if (restante > tiempoRestanteMs) tiempoRestanteMs = restante;
  }

  // ==========================================
  // REPORTE A PYTHON
  // ==========================================
  
  float maxRoto = max(roto1, roto2);
  float maxObs = max(obs1, obs2);
  
  float valNada = 0.0;
  if (nada1 > 0.5 && nada2 > 0.5) valNada = 1.0; 
  else if (nada1 > 0.5 || nada2 > 0.5) valNada = 0.5;

  // Truco visual: Si estamos en la cuenta regresiva de 5s, aseguramos que se vea ROTO
  if ((contandoFallaM1 || contandoFallaM2) && maxRoto < 0.5) {
      maxRoto = 1.0; // Forzamos que la barra roja se mantenga arriba durante los 5s
  }

  float valNormal = 1.0 - (maxRoto + maxObs + estadoBloqueo + valNada);
  if (valNormal < 0) valNormal = 0;

  float segundosRestantes = tiempoRestanteMs / 1000.0;
  if (segundosRestantes < 0) segundosRestantes = 0;

  Serial.print("DATA");
  Serial.print("|NORMAL:"); Serial.print(valNormal);
  Serial.print("|ROTO:"); Serial.print(maxRoto);
  Serial.print("|OBSTRUCCION:"); Serial.print(maxObs); 
  Serial.print("|BLOQUEO:"); Serial.print(estadoBloqueo);
  Serial.print("|NADA:"); Serial.print(valNada);
  Serial.print("|TIEMPO:"); Serial.print(segundosRestantes);
  Serial.println();
}

// --- GESTIÓN DE TIEMPOS (NUEVO) ---
void gestionarCuentaRegresiva() {
  // Revisar Motor 1
  if (contandoFallaM1) {
    if (millis() - inicioFallaM1 > TIEMPO_VISUALIZACION) {
      // Ya pasaron los 5 segundos -> AHORA SÍ APAGAMOS
      apagarMotor1();
      bloqueoMotor1 = true; 
      tiempoBloqueo1 = millis();
      contandoFallaM1 = false; // Reset de la cuenta
      Serial.println("! APAGADO FINAL M1 !");
    }
  }

  // Revisar Motor 2
  if (contandoFallaM2) {
    if (millis() - inicioFallaM2 > TIEMPO_VISUALIZACION) {
      apagarMotor2();
      bloqueoMotor2 = true;
      tiempoBloqueo2 = millis();
      contandoFallaM2 = false;
      Serial.println("! APAGADO FINAL M2 !");
    }
  }
}

void gestionarBloqueos() {
  if (bloqueoMotor1 && (millis() - tiempoBloqueo1 > CASTIGO)) {
    bloqueoMotor1 = false; encenderMotor1();
  }
  if (bloqueoMotor2 && (millis() - tiempoBloqueo2 > CASTIGO)) {
    bloqueoMotor2 = false; encenderMotor2();
  }
}

// --- AUXILIARES ---
bool leerVibracion(int direccionSensor) {
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
      uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
      Wire.beginTransmission(direccionSensor); Wire.write(0x3B); Wire.endTransmission(false);
      if (Wire.requestFrom(direccionSensor, 6, true) == 6) {
        int16_t AcX = Wire.read() << 8 | Wire.read();
        int16_t AcY = Wire.read() << 8 | Wire.read();
        int16_t AcZ = Wire.read() << 8 | Wire.read();
        features[ix + 0] = AcX * CONVERSION_G;
        features[ix + 1] = AcY * CONVERSION_G;
        features[ix + 2] = AcZ * CONVERSION_G;
      } else {
        Wire.flush(); return false;
      }
      while (micros() < next_tick) {}
  }
  return true;
}

bool motorEstaQuieto() {
  float acumulado_cambio = 0;
  for (int i = 3; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
    acumulado_cambio += abs(features[i] - features[i-3]);
  }
  float promedio_cambio = acumulado_cambio / (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 3);
  return (promedio_cambio < UMBRAL_OFF);
}

void analizarVibracion(float &outRoto, float &outObstruccion) {
  ei_impulse_result_t result = { 0 };
  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  run_classifier(&signal, &result, false);

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      String etiqueta = String(result.classification[ix].label);
      float valor = result.classification[ix].value;
      if (etiqueta == "ROTO") outRoto = valor;
      if (etiqueta == "OBSTRUCCION") outObstruccion = valor;
  }
}

void encenderMotor1() { digitalWrite(MOTOR1_IN1, HIGH); digitalWrite(MOTOR1_IN2, LOW); }
void apagarMotor1()   { digitalWrite(MOTOR1_IN1, LOW);  digitalWrite(MOTOR1_IN2, LOW); }
void encenderMotor2() { digitalWrite(MOTOR2_IN3, HIGH); digitalWrite(MOTOR2_IN4, LOW); }
void apagarMotor2()   { digitalWrite(MOTOR2_IN3, LOW);  digitalWrite(MOTOR2_IN4, LOW); }