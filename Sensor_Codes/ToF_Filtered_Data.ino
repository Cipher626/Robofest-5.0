#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;

#define HISTORY 5
#define RESOLUTION 64
#define THRESHOLD_MM 20
#define CLUSTER_MIN 3
#define HYSTERESIS_FRAMES 3

uint16_t history[RESOLUTION][HISTORY];
uint8_t histIndex = 0;
uint16_t baseline[RESOLUTION];
uint8_t persistence = 0;

uint16_t median(uint16_t arr[], uint8_t n) {
  uint16_t temp[HISTORY];
  memcpy(temp, arr, n * sizeof(uint16_t));
  for (int i = 0; i < n-1; i++) {
    for (int j = 0; j < n-i-1; j++) {
      if (temp[j] > temp[j+1]) {
        uint16_t t = temp[j];
        temp[j] = temp[j+1];
        temp[j+1] = t;
      }
    }
  }
  return temp[n/2];
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(400000);

  if (!myImager.begin()) {
    Serial.println("VL53L5CX not detected. Check wiring!");
    while (1) delay(10);
  }

  myImager.setResolution(64);       // 8x8 zones
  myImager.setRangingFrequency(10); // Hz
  myImager.startRanging();

  Serial.println("VL53L5CX initialized. Building baseline...");

  // Build baseline over 20 frames
  for (int f = 0; f < 20; f++) {
    while (!myImager.isDataReady()) delay(10);
    myImager.getRangingData(&measurementData);
    for (int i = 0; i < RESOLUTION; i++) {
      baseline[i] = measurementData.distance_mm[i];
    }
  }
  Serial.println("Baseline acquired.");
}

void loop() {
  if (myImager.isDataReady()) {
    myImager.getRangingData(&measurementData);

    int clusterCount = 0;

    // Central 2x2 pixels in 8x8 grid: indices 27,28,35,36
    int centralIndices[4] = {27, 28, 35, 36};

    for (int c = 0; c < 4; c++) {
      int i = centralIndices[c];
      uint16_t d = measurementData.distance_mm[i];

      // Range limit
      if (d < 50 || d > 2000) d = 0;

      // Median filter
      history[i][histIndex] = d;
      uint16_t filtered = median(history[i], HISTORY);

      // Delta vs baseline
      if (baseline[i] > 0 && filtered > 0) {
        int delta = baseline[i] - filtered;
        if (delta >= THRESHOLD_MM) {
          clusterCount++;
        }
      }
    }

    histIndex = (histIndex + 1) % HISTORY;

    // Hysteresis check
    if (clusterCount >= CLUSTER_MIN) {
      persistence++;
      if (persistence >= HYSTERESIS_FRAMES) {
        Serial.println("[DETECTED] Object present in central zone!");
        persistence = HYSTERESIS_FRAMES; // cap
      }
    } else {
      persistence = 0;
    }
  }
}