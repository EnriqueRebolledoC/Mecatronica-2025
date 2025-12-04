// ============================================================================
// SISTEMA DE CONTROL PID - ARREGLO MANUAL (SIN STEPPER.H)
// ============================================================================

#include <Wire.h>       

#define PI 3.14159265

// ============================================================================
// 1. DEFINICIÓN DE PINES DE MOTOR (DRIVER A3967 / A4988)
// ============================================================================
// Definimos los pines directamente:

const int DIR_1 = 2;
const int STEP_1 = 3;

const int DIR_2 = 4;
const int STEP_2 = 5;

// ============================================================================
// 2. DEFINICIÓN DE SENSOR
// ============================================================================
const int MPU_ADDR = 0x68; 
int16_t ax, ay, az;
int16_t gx, gy, gz;
float roll = 0;

// ============================================================================
// 3. VARIABLES PID Y CONTROL
// ============================================================================
float setpoint = 0.0;  

// --- Ganancias PID (Escaladas para Pasos/Segundo) ---
// Como ahora controlamos pulsos (frecuencia alta), los valores K deben ser mayores.
float Kp = 1100.0;   
float Ki = 0;   
float Kd = 25;    

float error = 0.0, error_prev = 0.0;
float integral = 0.0, derivada = 0.0;
float salidaPID = 0.0; // Esto será "Pasos por Segundo"

// Variables de Tiempo
unsigned long lastTimePID;
const unsigned long sampleTimePID = 10; // PID corre a 100Hz

// Variables para el control de motores (No Bloqueante)
unsigned long lastStepTime = 0; // Momento del último paso
long stepDelay = 0;             // Retardo entre pasos (micros)

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    // Configurar pines de motor como SALIDA
    pinMode(DIR_1, OUTPUT);
    pinMode(STEP_1, OUTPUT);
    pinMode(DIR_2, OUTPUT);
    pinMode(STEP_2, OUTPUT);

    // Serial opcional para debug
    Serial.begin(9600);

    // Iniciar Sensor
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);  
    Wire.write(0);     
    Wire.endTransmission(true);
    
    // ESPERA DE SEGURIDAD (5 seg)
    delay(5000); 

    lastTimePID = millis();
}

// ============================================================================
// LOOP PRINCIPAL
// ============================================================================
void loop() {
    
    // -------------------------------------------------------
    // TAREA 1: MOVER MOTORES (Se ejecuta lo más rápido posible)
    // -------------------------------------------------------
    moverMotoresContinuo();


    // -------------------------------------------------------
    // TAREA 2: CALCULAR PID (Se ejecuta cada 10ms)
    // -------------------------------------------------------
    unsigned long now = millis();
    if (now - lastTimePID >= sampleTimePID) {
        float dt = (now - lastTimePID) / 1000.0;
        lastTimePID = now; 

        // A. Leer Sensor
        readMPU6050();
        roll = atan2((float)ay, (float)az) * 180.0 / PI;
        
        // B. PID
        error = setpoint - roll; 

        float P = Kp * error;
        integral += error * dt;
        float I = Ki * integral;
        derivada = (error - error_prev) / dt;
        float D = Kd * derivada;

        salidaPID = P + I + D; 
        // salidaPID ahora representa PASOS POR SEGUNDO.
        
        error_prev = error;
        
        // Debug (opcional)
        // Serial.println(roll);
    } 
}

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

void readMPU6050() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); 
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);

    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
    Wire.read(); Wire.read(); 
    gx = Wire.read() << 8 | Wire.read();
    gy = Wire.read() << 8 | Wire.read();
    gz = Wire.read() << 8 | Wire.read();
}

// --- GESTOR DE MOVIMIENTO NO BLOQUEANTE ---
void moverMotoresContinuo() {
    
    // 1. Zona Muerta: Si la velocidad pedida es muy baja, no hacemos nada
    if (abs(salidaPID) < 20) { // Menos de 20 pasos/segundo es despreciable
        return;
    }

    // 2. Calcular Dirección
    // Si salida > 0 -> Adelante, Si < 0 -> Atrás
    // NOTA: Si un motor gira al revés, cambia HIGH por LOW en su pin DIR
    if (salidaPID > 0) {
        digitalWrite(DIR_1, LOW); 
        digitalWrite(DIR_2, LOW); 
    } else {
        digitalWrite(DIR_1, HIGH);
        digitalWrite(DIR_2, HIGH);
    }

    // 3. Calcular el retardo necesario entre pasos
    // Formula: 1.000.000 micros / pasos_por_segundo
    // Limitamos la velocidad máxima a aprox 4000 pasos/seg para evitar bloqueos
    float velocidad = abs(salidaPID);
    if (velocidad > 5000) velocidad = 5000;
    
    stepDelay = 1000000 / velocidad;

    // 4. Comprobar si es hora de dar un paso
    if (micros() - lastStepTime >= stepDelay) {
        lastStepTime = micros();
        
        // Dar un pulso a los pines STEP
        digitalWrite(STEP_1, HIGH);
        digitalWrite(STEP_2, HIGH);
        delayMicroseconds(2); // Pequeña pausa para que el driver detecte el pulso
        digitalWrite(STEP_1, LOW);
        digitalWrite(STEP_2, LOW);
    }
}