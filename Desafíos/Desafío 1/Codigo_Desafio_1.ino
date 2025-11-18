#include <WiFi.h>
#include <WebServer.h>

// ===================== Pines ============================

// Motor A (lado izquierdo)
const int AIN1 = 23;
const int AIN2 = 4;
const int PWMA = 22;

// Motor B (lado derecho)
const int BIN1 = 18;
const int BIN2 = 19;
const int PWMB = 21;

// STBY (habilita el TB6612FNG)
const int STBY = 5;

// LEDs
const int LED_1 = 32;
const int LED_2 = 26;
const int LED_3 = 33;
const int LED_4 = 25;

// ===================== PWM ============================

const int pwmFree = 1000;
const int pwmResolution = 8;
const int pwmChannelA = 0;
const int pwmChannelB = 1;

// Velocidad base (0–255)
int motorSpeed = 180;

// ===================== WiFi ============================

// Insertar el nombre de su red WiFi
const char* ssid = "A15";
const char* password = "qilegueialuz";

// Servidor web en el puerto 80
WebServer server(80);

// ===================== HTML (WASD + Flechas) ============================

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">
<style>
body { font-family: system-ui, Segoe UI, Roboto, Arial; margin: 16px; }
button { padding: 16px 18px; font-size: 18px; margin: 8px; border-radius: 10px; }
.grid { display: grid; gap: 8px; grid-template-columns: repeat(3, 120px); justify-content: center; margin-top: 16px; }
.kbd { padding: 2px 6px; border: 1px solid #ddd; border-radius: 6px; background: #ffffff; }
</style>
</head>
<body>
<h2>Desafio 1: "Fats & Rats"</h2>
<h3>Mecatrónica ME4250</h3>
<p>Usa <span class="kbd">W</span> <span class="kbd">A</span> <span class="kbd">S</span> <span class="kbd">D</span> o las flechas. <span class="kbd">Space</span> = Stop</p>

<div class="grid">
<div></div>
<button onclick="send('forward')"> ▼ Adelante</button>
<div></div>

<button onclick="send('left')">← Izquierda</button>
<button onclick="send('stop')"> □ Stop</button>
<button onclick="send('right')">➤ Derecha</button>

<div></div>
<button onclick="send('backward')"> ✅ Atrás</button>
<div></div>
</div>

<p>Velocidad: <span id="v">180</span>
<input id="spd" type="range" min="0" max="255" value="180" oninput="chgSpeed(this.value)">
</p>

<hr> <div class="grid" style="grid-template-columns: repeat(2, 1fr);">
<button onclick="send('function1')"> �� Función 1</button>
<button onclick="send('function2')">☺ Función 2</button>
</div>

<script>
const v = document.getElementById('v');
function send(cmd){ fetch('/' + cmd).catch(()=>{}); }
function chgSpeed(val){ v.textContent = val; fetch('/speed?val=' + val).catch(()=>{}); }

// Teclado: WASD y flechas
const mapDown = {
    'w':'forward','W':'forward','ArrowUp':'forward',
    's':'backward','S':'backward','ArrowDown':'backward',
    'a':'left','A':'left','ArrowLeft':'left',
    'd':'right','D':'right','ArrowRight':'right',
    ' ':'stop'
};

const pressed = new Set();
window.addEventListener('keydown',(e)=>{
    if(!(e.key in mapDown)) return;
    if(!pressed.has(e.key)){
    pressed.add(e.key);
    send(mapDown[e.key]);
    }
    e.preventDefault();
},{passive:false});
window.addEventListener('keyup',(e)=>{
    if(!(e.key in mapDown)) return;
    pressed.delete(e.key);
    if(e.key !== ' '){ send('stop'); }
    e.preventDefault();
},{passive:false});
</script>
</body>
</html>
)rawliteral";

// ================================ Util LEDs ================================

// apagar los LEDs
void ledsOff() {
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, LOW);
}

// encender los LEDs
void ledsOn() {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);
    digitalWrite(LED_4, HIGH);
}

void blinkLEDs(int times, int delayTime) {
    for(int i = 0; i < times; i++) {
    ledsOn();
    delay(delayTime);
    ledsOff();
    delay(delayTime);
    }
}

// LEDs del frente
void ledadelante() {
    ledsOff();
    // Allumer les LEDs avant
    digitalWrite(LED_3, HIGH);
    digitalWrite(LED_4, HIGH);
}

// LEDs de atras
void ledsatras() {
    ledsOff();
    // Allumer les LEDs arrière
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
}

// LEDs de la derecha
void ledDerecha() {
    ledsOff();
    // Allumer la LED droite
    digitalWrite(LED_4, HIGH);
}

// LEDs de la izquierda
void ledizquiera() {
    ledsOff();
    // Allumer la LED gauche
    digitalWrite(LED_3, HIGH);
}

// ===================== Motores ======================
void motorsStop() {
    // Detiene la dirección
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    // Detiene la velocidad (opcional, pero buena práctica)
    ledcWrite(PWMA, 0);
    ledcWrite(PWMB, 0);
}

void motorsForward() {
    // Motor A (Izquierdo) Adelante: AIN1 = HIGH, AIN2 = LOW
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    // Motor B (Derecho) Adelante: BIN1 = HIGH, BIN2 = LOW
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    // Aplica la velocidad configurada
    ledcWrite(PWMA, motorSpeed);
    ledcWrite(PWMB, motorSpeed);

    // encender los LEDs correspondientes
    ledadelante();
}

void motorsBackward() {
    // Motor A (Izquierdo) Atrás: AIN1 = LOW, AIN2 = HIGH
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    // Motor B (Derecho) Atrás: BIN1 = LOW, BIN2 = HIGH
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    // Aplica la velocidad configurada
    ledcWrite(PWMA, motorSpeed);
    ledcWrite(PWMB, motorSpeed);

    // encender los LEDs correspondientes
    ledsatras();
}

void motorsLeft() { // Gira sobre eje: Motor A (Izquierdo) Atrás, Motor B (Derecho) Adelante
    // Motor A (Izquierdo) Gira Atrás
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    // Motor B (Derecho) Gira Adelante
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    // Aplica la velocidad
    ledcWrite(PWMA, motorSpeed);
    ledcWrite(PWMB, motorSpeed);

    // encender los LEDs correspondientes
    ledizquiera();
}

void motorsRight() { // Gira sobre eje: Motor A (Izquierdo) Adelante, Motor B (Derecho) Atrás
    // Motor A (Izquierdo) Gira Adelante
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    // Motor B (Derecho) Gira Atrás
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    // Aplica la velocidad
    ledcWrite(PWMA, motorSpeed);
    ledcWrite(PWMB, motorSpeed);

    // encoder los LEDs correspondientes
    ledDerecha();
}

// ========================= Funciones extras ==========================

void function1() {
    Serial.println("Ejecutando Función 1");

    // Adelante
    motorsForward();
    ledadelante();
    delay(2000);

    // Girar a la izquierda
    motorsLeft();
    ledizquiera();
    delay(1000);

    // Atrás
    motorsBackward();
    ledsatras();
    delay(2000);

    // Parar + parpadeo
    motorsStop();
    blinkLEDs(3, 300);
}

// Función 2: Giros continuos tipo "baile"
void function2() {
    Serial.println("Ejecutando Función 2");

    for (int i=0; i<4; i++) {
    motorsRight();
    ledDerecha();
    delay(800);
    motorsLeft();
    ledizquiera();
    delay(800);
    }

    motorsStop();
    ledsOff();
}

// ===================== Setup =====================
void setup() {
    Serial.begin(115200);

    // definir el tipo de señal para los pines del driver y los leds 
    pinMode(STBY, OUTPUT);

    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMA, OUTPUT); // Aunque se usa con ledc, es buena práctica definirlo

    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT); // Igual que PWMA

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    pinMode(LED_4, OUTPUT);

    digitalWrite(STBY, HIGH); // habilitar driver

    // Configurar PWM en ESP32
    ledcAttach(PWMA, pwmFree, pwmResolution);
    ledcAttach(PWMB, pwmFree, pwmResolution);

    motorsStop();

    // Conexión WiFi (a tu router)
    WiFi.begin(ssid, password);

    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
    }
    Serial.println("\nConectado. IP: ");
    Serial.println(WiFi.localIP());

    // Rutas HTTP
    server.on("/", [](){ server.send(200, "text/html", html); });

    server.on("/forward", [](){ motorsForward(); server.send(200,"text/plain","Forward"); });
    server.on("/backward", [](){ motorsBackward(); server.send(200,"text/plain","Backward"); });
    server.on("/left", [](){ motorsLeft(); server.send(200,"text/plain","Left"); });
    server.on("/right", [](){ motorsRight(); server.send(200,"text/plain","Right"); });
    server.on("/stop", [](){ motorsStop(); server.send(200,"text/plain","Stop"); });

    // Nuevas rutas para las funciones 1 y 2
    server.on("/function1", [](){ function1(); server.send(200,"text/plain","Function 1 executed"); });
    server.on("/function2", [](){ function2(); server.send(200,"text/plain","Function 2 executed"); });

    // Cambiar velocidad desde el slider
    server.on("/speed", [](){
    if (server.hasArg("val")) {
        motorSpeed = constrain(server.arg("val").toInt(), 0, 255);
    }
    server.send(200, "text/plain", String(motorSpeed));
    });

    server.begin();
    Serial.println("Servidor HTTP listo.");
}

// ===================== Loop ======================
void loop() {
    server.handleClient();
}