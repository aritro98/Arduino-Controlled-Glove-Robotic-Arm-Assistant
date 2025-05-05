#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>

// Network credentials
const char* ssid = "KRS ARM BOT";
const char* password = "KRS 2024";

// Create WebServer object on port 80
WebServer server(80);

// Define servo parameters
#define NUM_SERVOS 6
#define INTERRUPT_PIN 26

// Define servo objects
Servo servos[NUM_SERVOS];

// Define servo GPIO pins
int servoPins[NUM_SERVOS] = {2, 4, 5, 18, 19, 23};

// Initial positions of the servos
int servoPositions[NUM_SERVOS] = {90, 90, 90, 90, 90, 90};

// OLED display parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Mode flag (0 for normal mode, 1 for gyro mode)
volatile int modeFlag = 0;

// HTML and JavaScript for the web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP Servo Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .slider-container {
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
      gap: 10px;
    }
    .slider-wrapper {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .slider {
      width: 180px;
      height: 300px;
      -webkit-appearance: slider-vertical; /* For Chrome/Safari */
      writing-mode: bt-lr; /* For Firefox */
    }
    .label-container {
      margin-bottom: 10px;
    }
    .value {
      font-size: 24px;
    }
  </style>
  <script>
    function updateServo(servo, value) {
      if (!updateServo.debounced) {
        updateServo.debounced = true;
        setTimeout(function() {
          updateServo.debounced = false;
        }, 100); // Debounce time in milliseconds

        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/update?servo=" + servo + "&value=" + value, true);
        xhr.send();
        document.getElementById("value" + servo).innerHTML = value;
      }
    }
  </script>
</head>
<body>
  <h2>ESP Servo Control</h2>
  <div class="slider-container">
    %SLIDERS%
  </div>
</body>
</html>
)rawliteral";

// Function to generate the sliders' HTML dynamically
String generateSliders() {
  String sliders = "";
  for (int i = 0; i < NUM_SERVOS; i++) {
    sliders += "<div class='slider-wrapper'><div class='label-container'><label for='servo" + String(i) + "'>Servo " + String(i + 1) + ":</label><span class='value' id='value" + String(i) + "'>" + String(servoPositions[i]) + "</span></div>";
    sliders += "<input type='range' min='0' max='180' value='" + String(servoPositions[i]) + "' class='slider' id='servo" + String(i) + "' oninput='updateServo(" + String(i) + ", this.value)'></div>";
  }
  return sliders;
}

// Replace %SLIDERS% placeholder in HTML with the generated sliders
String processor(const String& var) {
  if (var == "SLIDERS") {
    return generateSliders();
  }
  return String();
}

void handleRoot() {
  String html = index_html;
  html.replace("%SLIDERS%", generateSliders());
  server.send(200, "text/html", html);
}

void handleUpdate() {
  static unsigned long lastUpdateTime[NUM_SERVOS] = {0};
  const unsigned long debounceDelay = 100; // Minimum delay between updates in milliseconds

  if (server.hasArg("servo") && server.hasArg("value")) {
    int servo = server.arg("servo").toInt();
    int value = server.arg("value").toInt();

    // Check debounce time
    if (millis() - lastUpdateTime[servo] < debounceDelay) {
      server.send(400, "text/plain", "Debouncing");
      return;
    }

    // Update servo position
    if (servo >= 0 && servo < NUM_SERVOS) {
      servoPositions[servo] = value;
      servos[servo].write(value);
      Serial.printf("Servo %d updated to %d\n", servo, value);

      // Update last update time for debounce
      lastUpdateTime[servo] = millis();

      // Print all servo angles
      Serial.print("Current servo angles: ");
      for (int i = 0; i < NUM_SERVOS; i++) {
        Serial.print("Servo ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(servoPositions[i]);
        if (i < NUM_SERVOS - 1) {
          Serial.print(", ");
        }
      }
      Serial.println();

      // Update the OLED display
      updateDisplay();
    }
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

// Function to handle ESP-NOW message reception
void onDataReceive(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
  if (modeFlag == 1) { // Only process if in Gyro Mode
    // Print MAC address of the sender
    Serial.print("Sender MAC Address: ");
    for (int i = 0; i < 6; i++) {
      if (i > 0) Serial.print(":");
      Serial.print(info->src_addr[i], HEX);
    }
    Serial.println();

    // Print the received data
    String data = String((char*)incomingData);
    Serial.printf("Received Data: %s\n", data.c_str());

    // Parse the incoming string
    int angles[NUM_SERVOS];
    int index = 0;
    char token = strtok((char)incomingData, ",");
    while (token != nullptr && index < NUM_SERVOS) {
      angles[index++] = atoi(token);
      token = strtok(nullptr, ",");
    }

    // Update servo positions
    for (int i = 0; i < index; i++) {
      if (angles[i] >= 0 && angles[i] <= 180) {
        servoPositions[i] = angles[i];
        servos[i].write(angles[i]);
      }
    }

    // Update the OLED display
    updateDisplay();
  }
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

    // Wait for serial port to connect
  while (!Serial) {
    delay(100);
  }

  // Print the MAC address
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Attach servos to their respective GPIO pins
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoPositions[i]); // Set initial position to 90 degrees
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Set up the access point
  WiFi.softAP(ssid, password);

  // Print the IP address
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);

  // Start server
  server.begin();

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onDataReceive); // Register the receive callback

  // Set up GPIO 26 as input
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
}

void loop() {
  // Check GPIO 26 for mode switching
  static int lastButtonState = HIGH; // Assume button is not pressed
  int buttonState = digitalRead(INTERRUPT_PIN);

  // Check if the button is pressed (LOW)
  if (lastButtonState == HIGH && buttonState == LOW) {
    modeFlag = !modeFlag; // Toggle mode
    Serial.print("Mode changed to: ");
    Serial.println(modeFlag ? "Gyro" : "Normal");
    updateDisplay(); // Update display immediately on mode change
    if (modeFlag == 1) {
      WiFi.softAPdisconnect(true); // Disconnect and stop the AP
    } else {
      WiFi.softAP(ssid, password); // Recreate the AP
    }
    delay(500); // Debounce delay
  }

  lastButtonState = buttonState;

  // Handle client requests
  server.handleClient();
}

// Function to update the OLED display with servo positions and mode
void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Mode: ");
  display.println(modeFlag ? "Gyro" : "Normal");

  for (int i = 0; i < NUM_SERVOS; i++) {
    display.print("Servo ");
    display.print(i);
    display.print(": ");
    display.println(servoPositions[i]);
  }
  display.display();
}