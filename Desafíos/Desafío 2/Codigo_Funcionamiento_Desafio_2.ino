// #include <Servo.h>

/*
  Conexiones:
  - TCS3200: S0->A0, S1->A1, S2->A2, S3->A3, OUT->D2, VCC->5V, GND->GND
  - L298N: ENA=3 (PWM), IN1=8, IN2=9
  - Servos: servo1=D5, servo2=D4
*/

// ====== L298N ======
const int ENA = 3;
const int IN1 = 8;
const int IN2 = 9;

// ====== Servos ======
Servo servo1;  // D5
Servo servo2;  // D4

// ====== GY-31 (TCS3200) ======
const int S0 = A0;
const int S1 = A1;
const int S2 = A2;
const int S3 = A3;
const int SENSOR_OUT = 2;   // salida de frecuencia del sensor

// ====== Calibración (TODO: Pegar tus valores medidos) ======
int R_BLACK = 174, G_BLACK = 166, B_BLACK = 132;  // <-- "NEGRO" (R,G,B)
int R_WHITE = 35, G_WHITE = 34, B_WHITE = 30;  // <-- "BLANCO" (R,G,B)

// ====== Parámetros de operación ======
const uint8_t SERVO_NEUTRO = 90;
const int PWM_CINTA = 100;           // velocidad cinta (0–255)
const unsigned CINTA_MS = 10000;     // cuánto avanza la cinta por ciclo (ms)
const unsigned ESPERA_OBJ_MS = 2000; // “simula” que ponen objeto

// Umbrales de clasificación (0–255 en espacio normalizado luminosidad)
uint8_t T_dark  = 25;   // negro (muy oscuro)   <-- puedes ajustar
uint8_t T_white = 200;  // blanco (muy claro)   <-- puedes ajustar
uint8_t T_color = 15;   // diferencia mínima entre canal ganador y los demás

// ====== Helpers del motor ======
void motorStart(int pwm) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, pwm);
}
void motorStop() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// ====== Lectura del TCS3200 con pulseIn ======
unsigned long leerColor(bool s2, bool s3, uint8_t muestras = 4) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delayMicroseconds(300); // tiempo de asentamiento del filtro

  unsigned long acc = 0;
  for (uint8_t i = 0; i < muestras; i++) {
    unsigned long t = pulseIn(SENSOR_OUT, LOW, 25000UL); // timeout 25 ms
    if (t == 0) t = pulseIn(SENSOR_OUT, LOW, 25000UL);   // intento extra si hubo timeout
    acc += t;
  }
  return (muestras ? acc / muestras : acc);
}

// Mapea tiempo de pulso a intensidad 0–255 (invertido) con límites
int mapInvertidoConstrain(int t, int t_black, int t_white) {
  if (t_black == t_white) return 0;
  long val = map((long)t, (long)t_black, (long)t_white, 0, 255);
  return (int)constrain(val, 0, 255);
}

// Clasificación → devuelve etiqueta y “acción” (1 o 2) que activará luego los servos
const char* clasificarRGB(int R, int G, int B, int &accion) {
  accion = 0; // 0 = sin servo; 1 = acción 1; 2 = acción 2

  // Chequeo de extremos por luminancia aproximada
  int Y = (R*30 + G*59 + B*11) / 100;

  // ====== EJEMPLOS ya implementados ======
  // NEGRO: muy bajo en los tres canales
  if (Y < T_dark) {
    // Ejemplo: NEGRO -> SIN ACCIÓN (solo cinta)
    accion = 0;
    return "NEGRO";
  }


if (Y > T_white) {  // Blanco sigue derecho
    accion = 3;
    return "BLANCO"; 
  }

  // TODO(Alumno): ROJO
  // R dominante sobre G y B
  if (R > G + T_color && R > B + T_color) { 
    accion = 1; // Rojo -> ACCION 1
    return "ROJO"; 
  }

  // TODO(Alumno): MORADO (violeta)
  // R y B dominantes sobre G. Se chequea DESPUÉS de Rojo.
  if (R > G + T_color && B > G + T_color) { 
    accion = 2; // Morado -> ACCION 2
    return "MORADO"; 
  }

  // Otras posibilidades (gris, indefinido, etc.)
  if (abs(R-G) < 20 && abs(G-B) < 20) return "GRIS";

  return "INDEFINIDO";
}

void setup() {
  Serial.begin(9600);

  // Motor
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  motorStop();

  // Servos
  servo1.attach(5);
  servo2.attach(4);
  servo1.write(SERVO_NEUTRO);
  servo2.write(SERVO_NEUTRO);

  // Sensor TCS3200
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(SENSOR_OUT, INPUT);

// Sensibilidad: 100% (si se satura, cambiar a S0=HIGH, S1=LOW → 20%)
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  Serial.println("Desafío 2 - Plataforma de Sorting por Color lista.");
}

void loop() {
  // --- 1) Espera objeto (simulada) ---
  motorStop();
  servo1.write(SERVO_NEUTRO);
  servo2.write(SERVO_NEUTRO);
  Serial.println("\nEsperando objeto...");
  delay(ESPERA_OBJ_MS);

  // --- 2) Medición del color ---
  // ROJO: S2=LOW, S3=LOW
  unsigned long tR = leerColor(LOW, LOW);
  // VERDE: S2=HIGH, S3=HIGH
  unsigned long tG = leerColor(HIGH, HIGH);
  // AZUL: S2=LOW, S3=HIGH
  unsigned long tB = leerColor(LOW, HIGH);

  // Normalización invertida (0–255: 0 oscuro, 255 claro)
  int R = mapInvertidoConstrain(tR, R_BLACK, R_WHITE);
  int G = mapInvertidoConstrain(tG, G_BLACK, G_WHITE);
  int B = mapInvertidoConstrain(tB, B_BLACK, B_WHITE);

  int accion = 0;
  const char* etiqueta = clasificarRGB(R, G, B, accion);

  Serial.print("tR:"); Serial.print(tR);
  Serial.print(" tG:"); Serial.print(tG);
  Serial.print(" tB:"); Serial.print(tB);
  Serial.print(" | R:"); Serial.print(R);
  Serial.print(" G:"); Serial.print(G);
  Serial.print(" B:"); Serial.print(B);
  Serial.print(" -> "); Serial.println(etiqueta);

  // --- 3) Acción según color ---

  switch (accion) {
    case 1:
      // Mueve el servo 1 (D5) para desviar el objeto ROJO
      servo1.write(45); // Ajusta este ángulo (podría ser 0 o 180)
      Serial.println("ACCION 1: Desviando ROJO con Servo 1.");
      break;

    case 2:
      // Mueve el servo 2 (D4) para desviar el objeto MORADO
      servo2.write(45); // Ajusta este ángulo (podría ser 0 o 180)
      Serial.println("ACCION 2: Desviando MORADO con Servo 2.");
      break;
    
    case 3:
      // Mueve ambos servos para deviar el objeto BLANCO
      servo1.write(90);
      servo2.write(90);
      delay(100);
      servo1.write(0);
      servo2.write(0);
      break;

    default: // Acción 0 (NEGRO, BLANCO, AZUL, VERDE, etc.)
      Serial.println("Sin acción de servo (solo cinta).");
      break;
  }

  delay(400);

  // --- 4) Mover cinta ---
  Serial.println("Cinta en movimiento...");
  motorStart(PWM_CINTA);
  delay(CINTA_MS);
  motorStop();

  // --- 5) Reset servos ---
  servo1.write(SERVO_NEUTRO);
  servo2.write(SERVO_NEUTRO);

  Serial.println("Ciclo completo.");
  delay(1500);
}
