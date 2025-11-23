# 🔩 Manual de Armado y Puesta en Marcha

Este manual detalla los pasos esenciales para construir, conectar y poner en funcionamiento el **Robot Auto-Balancín**. Considerando todas las piezas listadas en `lista_componentes.xlsx` y los modelos 3D en `/Componentes 3D/`.

---

## 🛠️ 1. Armado Mecánico del Chasis

El chasis del robot se compone de varias piezas impresas en 3D o fabricadas (según el diseño en `/Componentes 3D/`), buscando la **simplicidad y facilidad de montaje**.

1.  **Ensamblaje de la Carcasa:** Ensamble las piezas de la estructura base del robot. (La carcasa está diseñada para ser **descomponible** para facilitar el reemplazo rápido de piezas).
2.  **Montaje de Motores:** Instale los **dos motores NEMA 17** en la parte inferior del chasis, asegurándose de que los ejes queden orientados hacia el exterior.
3.  **Montaje de Ruedas:** Fije las dos ruedas (diametro $9\text{ cm}$, ancho $\approx 2\text{ cm}$) a los ejes de los motores NEMA 17, utilizando acoples adecuados.
4.  **Distribución de Componentes:** Fije la placa **Arduino UNO ** y la caja de baterías/fuente de poder en los niveles superiores del chasis.

---

## 🔌 2. Conexiones Eléctricas y Electrónicas

**⚠️ Advertencia de Seguridad:** **No conecte la fuente de poder externa/batería y el cable USB (computador) simultáneamente.** De no hacerlo, podría dañar componentes o su computador.

### A. Conexión del Sensor MPU-6050

El sensor **MPU-6050 (GY521)** se conecta al Arduino vía el protocolo **I2C**. Instale el sensor cerca del centro de gravedad del robot.

| Pin MPU-6050 | Conexión Arduino UNO |
| :--- | :--- |
| **VCC** | $5\text{V}$ |
| **GND** | GND |
| **SCL** | A5 (Reloj I2C) |
| **SDA** | A4 (Datos I2C) |

### B. Conexión de los Drivers EasyDriver A3967

Los dos drivers **EasyDriver A3967** controlan los motores NEMA 17. Cada motor requiere un driver individual.

| Pin Driver A3967 | Conexión Arduino UNO | Función |
| :--- | :--- | :--- |
| **DIR** | Pin Digital (e.g., D2 y D4) | Define la **dirección** de giro del motor. |
| **STEP** | Pin Digital (e.g., D3 y D5) | Envía el **pulso de paso** del motor. |
| **PWR IN / M+** | Fuente de Poder Externa | Voltaje de alimentación del motor. |
| **GND** | GND | Tierra. |
| **Motor A/B** | Conectores del motor NEMA 17 | Conexión a las bobinas del motor. |

**Nota:** La corriente del driver A3967 es **ajustable**, lo que es ideal para los motores paso a paso.

---

## ⚙️ 3. Puesta en Marcha del Código y Calibración

### A. Carga del Código Base

1.  Asegúrese de tener instaladas las librerías necesarias (mínimo `Wire.h` y `Stepper.h`).
2.  Cargue el *firmware* en la placa Arduino.

### B. Calibración del Sensor MPU-6050

La calibración elimina el **offset** (desajuste) del giroscopio, que es crucial para obtener lecturas precisas del ángulo.

1.  **Posicionar:** Coloque el robot en una superficie **completamente nivelada e inmóvil**.
2.  **Iniciar Calibración:** Conecte el Arduino al PC (sin la batería externa) y abra el **Monitor Serial**.
3.  El sistema pedirá al usuario **presionar cualquier tecla + ENTER** para iniciar la lectura de las muestras.
4.  El código tomará **100 muestras** del giroscopio (ejes X, Y, Z) , calculará el promedio y guardará estos valores como *offsets* ($\mathbf{gx\_offset}$, $\mathbf{gy\_offset}$, $\mathbf{gz\_offset}$) para sustraerlos en futuras lecturas.

---

## 🔬 4. Sintonización del Control PID

El éxito del proyecto depende de la correcta sintonización de las constantes PID. El **ángulo de Roll** (o *Pitch*) es la entrada de error , y la **Salida del PID** es la velocidad (RPM) que se aplica a los motores. El **Setpoint** es $\mathbf{0.0\text{ grados}}$.

### A. Valores Iniciales

Los valores determinados en el **Avance 3** del proyecto para el equilibrio estable son un excelente punto de partida:

* **Ganancia Proporcional ($\mathbf{K_p}$):** $\mathbf{2.0}$. (Reacciona al error actual)
* **Ganancia Integral ($\mathbf{K_i}$):** $\mathbf{0.6}$. (Elimina el error acumulado)
* **Ganancia Derivativa ($\mathbf{K_d}$):** $\mathbf{8.1}$. (Predice el error futuro)

### B. Pruebas y Monitoreo

1.  **Monitoreo:** Utilice el **Serial Plotter** para graficar simultáneamente: **Roll, Setpoint y Salida del PID**.
2.  **Saturación y Anti-Windup:** El código ya incluye un mecanismo para limitar la salida del PID a un **rango máximo de velocidad** (e.g., $\pm 50.0\text{ RPM}$) y evitar el efecto *integral windup*.
3.  **Ajuste Fino:** Si el robot oscila demasiado o no logra el balance perfecto, modifique las constantes:
    * Aumentar $\mathbf{K_p}$ para hacerlo más reactivo (riesgo de oscilación).
    * Aumentar $\mathbf{K_d}$ para amortiguar la oscilación y mejorar la predicción.
    * Aumentar $\mathbf{K_i}$ si hay un error persistente en estado estacionario.

**¡La sintonización es un proceso iterativo!** Documente sus cambios y los resultados obtenidos.
