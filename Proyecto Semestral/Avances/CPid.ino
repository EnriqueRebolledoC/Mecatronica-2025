#include <Wire.h>
#define PI = 3.14159265  

///////////////////// CONFIGURACIÓN HARDWARE /////////////////////
const int PIN_LM35 = A0; // Sensor de Gyro521
const int ENA = 3;       // PWM del L298N
const int IN1 = 8;       // Control de giro
const int IN2 = 9;       // Control de giro
const float AREF_V = 5.0; // Referencia ADC
const int ADC_MAX = 1023; // Resolución 10 bits

///////////////////// PARÁMETROS PID /////////////////////
float Kp = 2.0;
float Ki = 0.6;
float Kd = 0.1;
float setpoint = 32.0;  // °C deseados

///////////////////// VARIABLES DEL CONTROL /////////////////////
float temperatura = 0.0;
float error = 0.0;
float error_prev = 0.0;
float integral = 0.0;
float derivada = 0.0;
float salida = 0.0;
int pwm = 0;

// Control de tiempo
unsigned long lastTime = 0;
const unsigned long sampleTime = 100; // ms (10 Hz)

///////////////////// FUNCIONES AUXILIARES /////////////////////
float leerTemperaturaLM35() {
  int lectura = analogRead(PIN_LM35);
  float voltaje = (lectura * AREF_V) / ADC_MAX;
  return voltaje * 100.0; // 10 mV/°C
}

void moverVentilador(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, pwmValue);
}

///////////////////// SETUP /////////////////////
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LM35, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  Serial.println("=== CONTROL PID DE TEMPERATURA ===");
  Serial.println("Graficar en Serial Plotter: Temperatura, Setpoint, PWM");
}

///////////////////// LOOP PRINCIPAL /////////////////////
void loop() {
  unsigned long now = millis();
  if (now - lastTime >= sampleTime) {
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;

    // 1) Leer sensor
    temperatura = leerTemperaturaLM35();

    // 2) Calcular error
    error = setpoint - temperatura;

    // 3) PID (manual)
    float P = Kp * error;
    integral += error * dt;
    float I = Ki * integral;
    derivada = (error - error_prev) / dt;
    float D = Kd * derivada;

    salida = P + I + D; // ->(error total)

    // 4) Saturación y anti-windup
    if (salida > 255) {
      salida = 255;
      integral -= error * dt; // Evita acumulación excesiva
    } else if (salida < 0) {
      salida = 0;
      integral -= error * dt;
    }

    // Otra opción
    //
    pwm = (int)salida;

    // 5) Aplicar al motor
    moverVentilador(pwm); 
    
    // 6) Mostrar datos (para el Serial Plotter)
    Serial.print(temperatura);
    Serial.print(",");
    Serial.print(setpoint);
    Serial.print(",");
    Serial.println(pwm);

    // 7) Guardar error anterior
    error_prev = error;
  }
}

temperatura: , setpoint: , pwm:
