#include <Wire.h>

// --- SENSOR VARIABLES ---
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;

// Filtered & Corrected Data
float AccX_F = 0, AccY_F = 0, AccZ_F = 0; 
float AngleRoll = 0, AnglePitch = 0;
float RateRoll, RatePitch, RateYaw;

// --- CALIBRATION VARIABLES ---
float RateCRoll, RateCPitch, RateCYaw;
float AccErrorX, AccErrorY, AccErrorZ;
int RateCnum;

// Timing
unsigned long LoopTimer;
float dt = 0.004;
unsigned long lastPrint = 0;   // for 1‑second print interval

// --- MPU SETUP ---
void setup_mpu() {
  Wire.beginTransmission(0x68); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission();
  Wire.beginTransmission(0x68); Wire.write(0x1A); Wire.write(0x05); Wire.endTransmission();
  Wire.beginTransmission(0x68); Wire.write(0x1B); Wire.write(0x08); Wire.endTransmission();
  Wire.beginTransmission(0x68); Wire.write(0x1C); Wire.write(0x10); Wire.endTransmission();
}

void read_mpu() {
  Wire.beginTransmission(0x68); Wire.write(0x3B); Wire.endTransmission();
  Wire.requestFrom(0x68, 14);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); 
  int16_t GyroXLSB = Wire.read() << 8 | Wire.read();
  int16_t GyroYLSB = Wire.read() << 8 | Wire.read();
  int16_t GyroZLSB = Wire.read() << 8 | Wire.read();

  // Convert to Physical Units
  AccX = (float)AccXLSB / 4096.0;
  AccY = (float)AccYLSB / 4096.0;
  AccZ = (float)AccZLSB / 4096.0;
  RateRoll = (float)GyroXLSB / 65.5;
  RatePitch = (float)GyroYLSB / 65.5;
  RateYaw = (float)GyroZLSB / 65.5;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();              // default SDA=21, SCL=22
  Wire.setClock(400000);
  delay(250);

  setup_mpu();

  // --- FULL CALIBRATION ROUTINE ---
  Serial.println("Calibrating Sensors... DO NOT MOVE!");
  
  for(RateCnum=0; RateCnum<2000; RateCnum++){
    read_mpu();
    RateCRoll += RateRoll; 
    RateCPitch += RatePitch; 
    RateCYaw += RateYaw;
    AccErrorX += AccX;
    AccErrorY += AccY;
    AccErrorZ += AccZ;
    delay(1);
  }
  
  RateCRoll /= 2000; RateCPitch /= 2000; RateCYaw /= 2000;
  AccErrorX /= 2000; AccErrorY /= 2000; AccErrorZ /= 2000;
  AccErrorZ = AccErrorZ - 1.0; // gravity fix

  Serial.println("Calibration Done!");
  Serial.print("Acc Z Offset: "); Serial.println(AccErrorZ);

  LoopTimer = micros();
}

void loop() {
  read_mpu();
  
  // Time Logic
  unsigned long current_time = micros();
  dt = (float)(current_time - LoopTimer) / 1000000.0;
  LoopTimer = current_time;

  // Apply Calibration
  RateRoll -= RateCRoll; RatePitch -= RateCPitch; RateYaw -= RateCYaw;
  AccX -= AccErrorX; AccY -= AccErrorY; AccZ -= AccErrorZ;

  // Low Pass Filter
  AccX_F = (AccX_F * 0.9) + (AccX * 0.1);
  AccY_F = (AccY_F * 0.9) + (AccY * 0.1);
  AccZ_F = (AccZ_F * 0.9) + (AccZ * 0.1);

  // Sensor Fusion
  float AccAngleRoll = atan(AccY / sqrt(AccX*AccX + AccZ*AccZ)) * 57.296;
  float AccAnglePitch = -atan(AccX / sqrt(AccY*AccY + AccZ*AccZ)) * 57.296;

  AngleRoll = 0.98 * (AngleRoll + RateRoll * dt) + 0.02 * AccAngleRoll;
  AnglePitch = 0.98 * (AnglePitch + RatePitch * dt) + 0.02 * AccAnglePitch;

  // --- PRINT TO SERIAL ONCE PER SECOND ---
  if (millis() - lastPrint >= 1000) {
    Serial.print("Roll: "); Serial.print(AngleRoll, 1);
    Serial.print(" | Pitch: "); Serial.print(AnglePitch, 1);
    Serial.print(" | AccX: "); Serial.print(AccX_F * 9.81, 2);
    Serial.print(" | AccY: "); Serial.print(AccY_F * 9.81, 2);
    Serial.print(" | AccZ: "); Serial.println(AccZ_F * 9.81, 2);
    lastPrint = millis();
  }
}