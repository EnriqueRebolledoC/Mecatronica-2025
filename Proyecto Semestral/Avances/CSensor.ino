#include <Wire.h>
#define PI 3.14159265

const int MPU_ADDR = 0x68;

int16_t ax, ay, az;
int16_t gx, gy, gz;

int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;

float roll = 0, pitch = 0, yaw = 0;
unsigned long lastTime;
float dt;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Power management
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("=== MPU-6050 Calibration ===");
  Serial.println("Place the sensor still and press any key + ENTER to start...");
  while (!Serial.available());  // Wait for user input
  while (Serial.available()) Serial.read();  // Clear input buffer

  calibrateGyro();
  Serial.println("Calibration complete!");

  lastTime = millis();
}

void loop() {
  readMPU6050();

  unsigned long now = millis();
  dt = (now - lastTime) / 1000.0;
  lastTime = now;

  float gyroZ = (gz - gz_offset) / 131.0;
  yaw += gyroZ * dt;

  // Convert int16_t to float
  float ax_f = (float)ax;
  float ay_f = (float)ay;
  float az_f = (float)az;

  // Avoid division by zero
  float denominator = sqrt(ay_f * ay_f + az_f * az_f);
  if (denominator < 0.0001) denominator = 0.0001;

  // Compute angles in degrees
  roll = atan2(ay_f, az_f) * 180.0 / PI;
  pitch = atan2(-ax_f, denominator) * 180.0 / PI;

  // Print angles
  Serial.print("Roll: "); Serial.print(roll, 2);
  Serial.print(" | Pitch: "); Serial.print(pitch, 2);
  Serial.print(" | Yaw: "); Serial.println(yaw, 2);

  delay(100);
}

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
