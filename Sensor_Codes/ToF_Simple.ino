#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);   // SDA=21, SCL=22
  Wire.setClock(400000);

  if (!myImager.begin()) {
    Serial.println("VL53L5CX not detected. Check wiring!");
    while (1) delay(10);
  }

  myImager.setResolution(64);       // 4x4 zones
  myImager.setRangingFrequency(10); // Hz
  myImager.setIntegrationTime(20);
  myImager.startRanging();
}

void loop() {
  if (myImager.isDataReady()) {
    myImager.getRangingData(&measurementData);
    if(measurementData.distance_mm[i] < 2000){
    for (int i = 0; i < myImager.getResolution(); i++) {
      Serial.print(measurementData.distance_mm[i]);
      Serial.print(" ");
    }
    }
    Serial.println();
  }
}