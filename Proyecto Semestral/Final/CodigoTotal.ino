// Librerias //

#include <Wire.h> // Para comunicacion del sensor
#define PI 3.14159265  
#include <Stepper.h> // Para control de motores

////// DEFINICION DE MOTOR /////

const int stepsPerRevolution = 200;

// Motor 1 en pines STEP=3, DIR=2
Stepper stepper1(stepsPerRevolution, 3, 2);

// Motor 2 en pines STEP=5, DIR=4
Stepper stepper2(stepsPerRevolution, 5, 4);

////// DEFINICION DE SENSOR /////

const int MPU_ADDR = 0x68;

int16_t ax, ay, az;
int16_t gx, gy, gz;

int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;

float roll = 0, pitch = 0, yaw = 0;
unsigned long lastTime;
float dt;

////// DEFINICION DE PID /////


///////////////////// PARÁMETROS DEL CONTROL PID /////////////////////
// Setpoint: El ángulo deseado (para un sistema auto-balanceante es 0 grados)
float setpoint = 0.0;  

// Ganancias PID (Ajustables)
float Kp = 1.0;         // Proporcional: reacciona al error actual
float Ki = 0.6;         // Integral: elimina el error acumulado
float Kd = 0.1;         // Derivativa: predice el error futuro

///////////////////// VARIABLES DEL CÁLCULO PID /////////////////////

// Variables de cálculo del PID
float error = 0.0;
float error_prev = 0.0;
float integral = 0.0;
float derivada = 0.0;
float salida = 0.0;    // La salida del PID es la corrección a aplicar (torque/velocidad)

// Control de tiempo para el cálculo del PID y del sensor
const unsigned long sampleTime = 10; // ms (100 Hz, un buen punto de partida para control)

///////////////////// VARIABLES DE CONTROL DE MOTOR /////////////////////
// La salida del PID se convierte en la velocidad/dirección de los motores.
int velocidadMotor1 = 0;
int velocidadMotor2 = 0;


////////// VOID SETUP /////////////////////

void setup() {
    // 1. Configuración de Comunicación Serial
    Serial.begin(9600); 
    
    // SOLUCIÓN A: Retardo y Limpieza Forzada para evitar basura inicial
    delay(1000); // Esperar 1 segundo para que la comunicación se estabilice
    while (Serial.available()) { // Limpiar cualquier dato de ruido que haya llegado
        Serial.read(); 
    }

    Serial.println("=== SISTEMA DE CONTROL PID DE EQUILIBRIO ===");

    // 2. Inicialización del MPU-6050 (I2C)
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); // Power management register
    Wire.write(0);    // Wake up MPU-6050
    Wire.endTransmission(true);

    // 3. Calibración del Sensor
    Serial.println("=== MPU-6050 Calibration ===");
    Serial.println("Place the sensor still and press any key + ENTER to start...");
    
    // SOLUCIÓN B: Bloqueo de espera simplificado
    while (!Serial.available()); // Esperar solo un carácter
   // while (Serial.available()) Serial.read(); // Limpiar el buffer después de la entrada (incluyendo el Enter)
   // calibrateGyro();
    Serial.println("Calibration complete!");

    // 4. Inicialización de Motores
    stepper1.setSpeed(0); // en RPM
    stepper2.setSpeed(0);

    // 5. Configuración de tiempo final
    lastTime = millis();
    Serial.println("Graficar en Serial Plotter: Roll, Setpoint, Salida");
}


/// VOID LOOPS ////////////////////////////////

void loop() {
    
    // --- CONTROL DE TIEMPO (Solo se ejecuta cada 'sampleTime' ms) ---
    unsigned long now = millis();
    
    if (now - lastTime >= sampleTime) { 
        // Calcular dt para el PID y reajustar el tiempo de la última ejecución
        float dt = (now - lastTime) / 1000.0; // Diferencia de tiempo en segundos
        lastTime = now; // Reinicia el contador de tiempo para el siguiente ciclo

        // --- 1. LECTURA Y CÁLCULO DE ÁNGULO (SENSOR) ---
        readMPU6050();

        // Convertir int16_t a float
        float ax_f = (float)ax;
        float ay_f = (float)ay;
        float az_f = (float)az;

        // Cálculo de Roll (Input al PID)
        float denominator = sqrt(ay_f * ay_f + az_f * az_f);
        if (denominator < 0.0001) denominator = 0.0001;
        roll = atan2(ay_f, az_f) * 180.0 / PI; 
        
        // (Opcional, el Pitch y Yaw del sensor original)
        // pitch = atan2(-ax_f, denominator) * 180.0 / PI;
        // float gyroZ = (gz - gz_offset) / 131.0;
        // yaw += gyroZ * dt;

        // --- 2. CÁLCULO PID ---

        // Calcular error: cuánto nos desviamos del setpoint (0 grados)
        error = setpoint - roll; 

        // Cálculo de las componentes PID
        float P = Kp * error;
        integral += error * dt;
        float I = Ki * integral;
        derivada = (error - error_prev) / dt;
        float D = Kd * derivada;

        salida = P + I + D; // La salida es el valor de corrección (velocidad RPM)

        // --- 3. SATURACIÓN y ANTI-WINDUP ---
        // Utilizamos el rango de RPM que definimos antes (ej. -50 a 50)
        const float MAX_OUTPUT_SPEED = 50.0; 
        
        if (salida > MAX_OUTPUT_SPEED) {
            salida = MAX_OUTPUT_SPEED;
            integral -= error * dt; // Anti-windup
        } else if (salida < -MAX_OUTPUT_SPEED) { 
            salida = -MAX_OUTPUT_SPEED;
            integral -= error * dt; // Anti-windup
        }

        // --- 4. APLICAR AL MOTOR ---
        aplicarControlMotor(salida); 

        // --- 5. MONITOREO SERIAL ---
        Serial.print(roll);
        Serial.print(",");
        Serial.print(setpoint);
        Serial.print(",");
        Serial.println(salida);
        
        // --- 6. Guardar error anterior ---
        error_prev = error;
    } 
    
}


// VOIDS AUXILIARES //


void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Skip temperature
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  long sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    readMPU6050();
    sumX += gx;
    sumY += gy;
    sumZ += gz;
    delay(5);
  }

  gx_offset = sumX / samples;
  gy_offset = sumY / samples;
  gz_offset = sumZ / samples;
}

// Función de actuación para los motores paso a paso
void aplicarControlMotor(float salida) {
  
  // Rango máximo de velocidad (en RPM)
  const float MAX_RPM = 50.0; 
  
  // Limita la salida PID (Constrain)
  salida = constrain(salida, -MAX_RPM, MAX_RPM); 
  
  // La salida PID (float) se convierte en la velocidad (int)
  int velocidadMotor = (int)salida; 

  // Aplicar la velocidad (positiva/negativa indica dirección)
  stepper1.setSpeed(velocidadMotor);
  stepper2.setSpeed(velocidadMotor);

  // Mover un paso para que el motor empiece a girar a la nueva velocidad
  stepper1.step(1); 
  stepper2.step(1); 
}
