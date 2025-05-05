#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <esp_now.h>
#include <math.h>

// Define the mode pin
#define MODE_PIN 26

// Create an instance of the MPU6050
MPU6050 mpu;

// Define the constants for angle calculation
const float DEG_PER_RAD = 180.0 / M_PI;

// Define servo range
const int SERVO_MIN = 0;
const int SERVO_MAX = 180;

// Track previous and current servo angles
int previousServo1 = 90;
int previousServo2 = 90;
int previousServo3 = 90;
int previousServo4 = 90;
int previousServo5 = 90;
int previousServo6 = 90;

// Function to calculate pitch, roll, and yaw from accelerometer and gyroscope values
void calculateOrientation(float &pitch, float &roll, float &yaw) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float aX = ax / 16384.0; // Sensitivity scale factor
  float aY = ay / 16384.0;
  float aZ = az / 16384.0;

  pitch = atan2(aY, sqrt(aX * aX + aZ * aZ)) * DEG_PER_RAD;
  roll = atan2(-aX, aZ) * DEG_PER_RAD;

  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  float gyroX = gx / 131.0;
  float gyroY = gy / 131.0;
  float gyroZ = gz / 131.0;

  // Simple integration to get yaw (change in angle)
  static float previousYaw = 0.0;
  yaw = previousYaw + gyroZ * 0.01;
  previousYaw = yaw;
}

// Function to map angle to servo range
int mapToServoRange(float angle) {
  return constrain(map(angle, -90, 90, SERVO_MIN, SERVO_MAX), SERVO_MIN, SERVO_MAX);
}

// Function to format and send servo angles via ESP-NOW
void sendServoAngles() {
  float pitch, roll, yaw;

  // Calculate pitch, roll, and yaw
  calculateOrientation(pitch, roll, yaw);

  // Map pitch, roll, and yaw to servo angles
  int currentServo1 = mapToServoRange(pitch);
  int currentServo2 = mapToServoRange(roll);
  int currentServo3 = mapToServoRange(yaw);

  String dataString;

  static bool modeFlag = false; // Ensure modeFlag is static to retain its value
  if (modeFlag) {
    // Mode 1: Send (current1, current2, current3, previous4, previous5, previous6)
    dataString = "(" + String(currentServo1) + "," + String(currentServo2) + "," + String(currentServo3) + "," +
                 String(previousServo4) + "," + String(previousServo5) + "," + String(previousServo6) + ")";

    // Update previous values for Mode 1 after sending
    previousServo4 = currentServo1; // Update previous values for Mode 1
    previousServo5 = currentServo2;
    previousServo6 = currentServo3;

    // Print the values being sent in Mode 1
    Serial.print("Mode 1: ");
    Serial.print(currentServo1); Serial.print(", ");
    Serial.print(currentServo2); Serial.print(", ");
    Serial.print(currentServo3); Serial.print(", ");
    Serial.print(previousServo4); Serial.print(", ");
    Serial.print(previousServo5); Serial.print(", ");
    Serial.println(previousServo6);

  } else {
    // Mode 2: Send (previous1, previous2, previous3, current4, current5, current6)
    dataString = "(" + String(previousServo1) + "," + String(previousServo2) + "," + String(previousServo3) + "," +
                 String(currentServo1) + "," + String(currentServo2) + "," + String(currentServo3) + ")";

    // Update previous values for Mode 2 after sending
    previousServo1 = previousServo1; // Keep the previous value
    previousServo2 = previousServo2; // Keep the previous value
    previousServo3 = previousServo3; // Keep the previous value

    // Update current values for Mode 2
    previousServo4 = currentServo1; // Update previous values for Mode 2
    previousServo5 = currentServo2;
    previousServo6 = currentServo3;

    // Print the values being sent in Mode 2
    Serial.print("Mode 2: ");
    Serial.print(previousServo1); Serial.print(", ");
    Serial.print(previousServo2); Serial.print(", ");
    Serial.print(previousServo3); Serial.print(", ");
    Serial.print(currentServo1); Serial.print(", ");
    Serial.print(currentServo2); Serial.print(", ");
    Serial.println(currentServo3);
  }

  // Print the formatted string to Serial Monitor
  Serial.print("Sending data: ");
  Serial.println(dataString);

  // Send the formatted string as a byte array
  esp_err_t result = esp_now_send(NULL, (uint8_t*)dataString.c_str(), dataString.length());
  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
  }
}

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Success");
  } else {
    Serial.println("Error");
  }
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize the MPU6050
  Wire.begin();
  mpu.initialize();

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // Add the peer ESP32 (replace with your receiver's MAC address)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6); // Broadcast address
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Set up the mode pin
  pinMode(MODE_PIN, INPUT_PULLUP);
}

void loop() {
  // Check the mode pin
  bool modeState = digitalRead(MODE_PIN);

  // Toggle mode on falling edge (button press)
  static bool lastModeState = HIGH;
  if (lastModeState == HIGH && modeState == LOW) {
    // Toggle mode
    static bool modeFlag = false; // Make sure modeFlag is static
    modeFlag = !modeFlag;
    Serial.print("Mode changed to: ");
    Serial.println(modeFlag ? "Mode 1" : "Mode 2");
    delay(500); // Debounce delay
  }
  lastModeState = modeState;

  // Send servo angles
  sendServoAngles();

  // Delay to control sending rate
  delay(200); // Adjust as needed
}