# 🚀 Proyecto Final: Robot Auto-Balancín (Self-Balancing Robot)

El objetivo de este proyecto es diseñar, construir y programar un robot móvil capaz de mantenerse en equilibrio de forma autónoma utilizando un **Controlador Proporcional-Integral-Derivativo (PID)**.

## 🎯 Componentes Clave y Control

* **Sistema de Detección de Inclinación:** Sensor IMU **MPU-6050** (Acelerómetro y Giroscopio) para medir el ángulo de inclinación (*Pitch*).
* **Actuación:** Motores de paso a paso controlados por *drivers* **EasyDriver A3967**.
* **Control Esencial:** El balance se logra mediante un **Algoritmo PID** que utiliza la lectura del ángulo como entrada de error y ajusta la velocidad/dirección de los motores como salida.

### **Fase de Implementación (Avance 3)**
La clave del éxito en la replicación es la correcta **sintonización del control PID**. El código debe incluir las ganancias $K_p$, $K_i$ y $K_d$ que se determinaron experimentalmente para el sistema.

---

## 🔩 Información para la Replicación

Para que cualquier persona pueda replicar el robot, toda la documentación esencial se encuentra organizada en las siguientes subcarpetas:

### 1. Listado Completo de Piezas y Materiales (`bill_of_materials.xlsx`)

Este archivo contiene la lista detallada de todos los componentes electrónicos y mecánicos.

| Categoría | Ejemplo de Componente | Notas |
| :--- | :--- | :--- |
| **Microcontrolador** | ESP32 o Arduino Mega | Debe ser capaz de manejar la lógica de control. |
| **Sensores** | MPU-6050 (GY-521) | Esencial para la medición de ángulo y velocidad angular. |
| **Actuadores** | Motores Paso a Paso (NEMA) | Los motores deben proporcionar suficiente torque. |
| **Drivers** | EasyDriver A3967 | Utilizados para controlar la corriente y pasos de los motores. |

### 2. Modelos 3D del Sistema (`/componentes_3d/`)

Aquí se encuentran los archivos de diseño CAD utilizados para fabricar el *chasis* del robot y los soportes de componentes.
* **Archivos:** `.STL`, `.STEP` o nativos de SolidWorks/Fusion 360.

### 3. Esquemáticos de Conexiones (`/esquematicos/`)

Esta carpeta incluye diagramas de conexión esenciales.
* **Conexiones Clave:**
    * Microcontrolador $\leftrightarrow$ MPU-6050 (Vía I2C).
    * Microcontrolador $\leftrightarrow$ EasyDriver (Pines de STEP/DIR).
    * EasyDriver $\leftrightarrow$ Motores Paso a Paso.
* **Documentos:** Diagramas de Fritzing, KiCad o diagramas de bloques simplificados.

### 4. Manual de Armado y Puesta en Marcha

Este manual te guiará paso a paso:
1.  **Montaje Mecánico:** Instrucciones de ensamblaje del chasis y motores.
2.  **Conexión Electrónica:** Sigue los diagramas en `/esquematicos/`.
3.  **Carga de Código:** Instrucciones para cargar el *firmware* inicial en `/codigo/`.
4.  **Calibración y Sintonización:** Pasos para obtener la compensación del MPU-6050 y la metodología para sintonizar las constantes **PID** y lograr el balance estable.
