// #include <Arduino.h>
// #include <ESP32Servo.h>

// // Servo setup
// Servo myServo;
// const int servoPin = 18;

// // Ultrasonic setup
// const int trigPin = 5;
// const int echoPin = 4;

// // Shared distance variable
// volatile float currentDistance = 0.0;

// // Task handles
// TaskHandle_t TaskServo;
// TaskHandle_t TaskUltrasonic;

// void setup() {
//   Serial.begin(115200);

//   // Servo initialization
//   myServo.setPeriodHertz(50);
//   myServo.attach(servoPin);

//   // Ultrasonic pin modes
//   pinMode(trigPin, OUTPUT);
//   pinMode(echoPin, INPUT);

//   // Create tasks
//   xTaskCreatePinnedToCore(servoTask, "Servo Task", 2048, NULL, 1, &TaskServo, 0);
//   xTaskCreatePinnedToCore(ultrasonicTask, "Ultrasonic Task", 2048, NULL, 1, &TaskUltrasonic, 1);
// }

// void loop() {
//   // FreeRTOS handles everything
// }

// void servoTask(void *pvParameters) {
//   while (true) {
//     if (currentDistance > 30.0) {
//       // Sweep from 0 to 180
//       for (int angle = 0; angle <= 180; angle += 10) {
//         myServo.write(angle);
//         vTaskDelay(200 / portTICK_PERIOD_MS);
//       }
//       // Sweep back from 180 to 0
//       for (int angle = 180; angle >= 0; angle -= 10) {
//         myServo.write(angle);
//         vTaskDelay(200 / portTICK_PERIOD_MS);
//       }
//     } else {
//       // Hold position
//       myServo.write(90);
//       vTaskDelay(500 / portTICK_PERIOD_MS);
//     }
//   }
// }

// void ultrasonicTask(void *pvParameters) {
//   while (true) {
//     digitalWrite(trigPin, LOW);
//     delayMicroseconds(2);
//     digitalWrite(trigPin, HIGH);
//     delayMicroseconds(10);
//     digitalWrite(trigPin, LOW);

//     long duration = pulseIn(echoPin, HIGH);
//     float distance = duration * 0.034 / 2;

//     currentDistance = distance; // Update shared variable

//     Serial.printf("Distance: %.2f cm\n", distance);
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//   }
// }


// -----------------------------------------------------------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <ESP32Servo.h>

// Servo setup
Servo myServo;
const int servoPin = 18;

// Ultrasonic setup
const int trigPin = 5;
const int echoPin = 4;

// Shared distance variable
volatile float currentDistance = 0.0;

// Servo angle tracker
volatile int currentAngle = 0;
bool increasing = true;

// Task handles
TaskHandle_t TaskServo;
TaskHandle_t TaskUltrasonic;

void setup() {
  Serial.begin(115200);

  // Servo initialization
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin);
  myServo.write(currentAngle);

  // Ultrasonic pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Create tasks
  xTaskCreatePinnedToCore(servoTask, "Servo Task", 2048, NULL, 1, &TaskServo, 0);
  xTaskCreatePinnedToCore(ultrasonicTask, "Ultrasonic Task", 2048, NULL, 1, &TaskUltrasonic, 1);
}

void loop() {
  // FreeRTOS handles everything
}

void servoTask(void *pvParameters) {
  while (true) {
    if (currentDistance > 30.0) {
      // Move one step in current direction
      myServo.write(currentAngle);

      if (increasing) {
        currentAngle += 10;
        if (currentAngle >= 180) {
          currentAngle = 180;
          increasing = false;
        }
      } else {
        currentAngle -= 10;
        if (currentAngle <= 0) {
          currentAngle = 0;
          increasing = true;
        }
      }
    }
    // Wait before next step or check
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void ultrasonicTask(void *pvParameters) {
  while (true) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    float distance = duration * 0.034 / 2;

    currentDistance = distance;

    Serial.printf("Distance: %.2f cm\n", distance);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}