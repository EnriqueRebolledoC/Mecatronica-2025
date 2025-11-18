// /*
  Calibración Sensor de Color TCS3200 / GY-31 (pines analógicos)
  Conexiones:
  S0 -> A0
  S1 -> A1
  S2 -> A2
  S3 -> A3
  OUT -> D2
  VCC -> 5V
  GND -> GND
*/

#define S0 A0        // Control frecuencia de salida
#define S1 A1        // Control frecuencia de salida
#define S2 A2        // Selección de filtro de color (R/G/B)
#define S3 A3        // Selección de filtro de color (R/G/B)
#define sensorOut 2  // Pin de salida del sensor (digital)

int redFreq, greenFreq, blueFreq;

// Lee un color con opción de promediar varias lecturas para estabilidad
unsigned long leerColor(bool s2State, bool s3State, uint8_t muestras = 3) {
  digitalWrite(S2, s2State);
  digitalWrite(S3, s3State);

  // Pequeño tiempo para que el filtro cambie establemente
  delayMicroseconds(300);

  unsigned long acc = 0;
  for (uint8_t i = 0; i < muestras; i++) {
    // pulseIn mide duración del pulso LOW (usamos timeout para evitar bloqueos)
    unsigned long t = pulseIn(sensorOut, LOW, 25000UL); // 25 ms timeout
    if (t == 0) {
      // Si no hubo pulso a tiempo, intenta una vez más
      t = pulseIn(sensorOut, LOW, 25000UL);
    }
    acc += (t > 0 ? t : 0);
  }
  return (muestras ? acc / muestras : acc);
}

void setup() {
  Serial.begin(9600);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // Escala al 100% para máxima sensibilidad
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  Serial.println("Calibración de sensor de color (GY-31) iniciada...");
  Serial.println("Coloca NEGRO y luego BLANCO para registrar R, G, B mínimos y máximos.");
}

void loop() {
  // Filtro ROJO (S2=LOW, S3=LOW)
  redFreq = leerColor(LOW, LOW);

  // Filtro VERDE (S2=HIGH, S3=HIGH)
  greenFreq = leerColor(HIGH, HIGH);

  // Filtro AZUL (S2=LOW, S3=HIGH)
  blueFreq = leerColor(LOW, HIGH);

  Serial.print("R: "); Serial.print(redFreq);
  Serial.print("  G: "); Serial.print(greenFreq);
  Serial.print("  B: "); Serial.println(blueFreq);

  delay(500); }