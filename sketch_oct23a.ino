#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

// Define pins for the HC-SR04
const int trigPin = 4;   // Trig pin connected to (D4)
const int echoPin = 2;   // Echo pin connected to (D2)
const int windSpeedPin = 23; 

// Variables for calculating distance
long duration;
float distanceCm;
float windSpeed;
volatile unsigned long pulseCount = 0;

// Interrupt service routine to count pulses
void IRAM_ATTR countPulse() {
    pulseCount++;
}

// SSID and password for the ESP32 AP
const char *ssid = "ESP32-Access-Point";  // Replace with your desired SSID
const char *password = "123456789";       // Replace with your desired password

// Create a web server on port 80
WebServer server(80);

// Function to read distance
void readDistance() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10us HIGH pulse to the Trig pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin and measure the duration of the pulse in microseconds
  duration = pulseIn(echoPin, HIGH, 10000); // 10ms timeout

  // If no echo is received (timeout), duration will be 0
  if (duration == 0) {
    Serial.println("No echo received, check wiring or sensor");
    distanceCm = -1; // Use -1 to denote error
  } else {
    // Calculate the distance in centimeters (sound speed = 34300 cm/s)
    distanceCm = (duration * 0.034) / 2;
  }
}

// Function to read wind speed
void readWindSpeed() {
  // Read pulse count every second
  unsigned long pulseCopy;
  noInterrupts(); // Disable interrupts while copying
  pulseCopy = pulseCount; // Copy pulse count
  pulseCount = 0; // Reset pulse count
  interrupts(); // Re-enable interrupts

  // Convert pulse count to wind speed (example conversion)
  windSpeed = pulseCopy * (0.0034); // Replace 0.01 with your conversion factor
}

// Combined handler for both distance and wind speed data
void handleData() {
  // Get the 'type' parameter from the request (if it exists)
  String type = server.arg("type");

  if (type == "ultra") {
    // Handle distance data
    readDistance(); // Read distance from ultrasonic sensor
    String dataultra = String(distanceCm) + " cm"; // Prepare response
    server.send(200, "text/plain", dataultra); // Send distance data
  } else if (type == "wind") {
    // Handle wind speed data
    readWindSpeed(); // Read wind speed
    String datawind = String(windSpeed) + " m/s"; // Prepare response
    server.send(200, "text/plain", datawind); // Send wind speed data
  } else {
    // Invalid type or no type provided
    server.send(400, "text/plain", "Invalid request"); // Send error message
  }
}

void handleRoot() {
  // Create an HTML response
  String html = "<!DOCTYPE html>";
  html += "<html lang=\"en\">";
  html += "<head>";
  html += "    <meta charset=\"UTF-8\">";
  html += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "    <title>Evaporation Meter</title>";
  html += "    <!-- Fonts -->";
  html += "    <style>";
  html += "        body {";
  html += "            background: linear-gradient(135deg, #2b5876, #4e4376, #f64c72);";
  html += "            font-family: Arial, sans-serif;";
  html += "            color: #ffffff;";
  html += "            margin: 0;";
  html += "            padding: 0;";
  html += "            height: 100vh;";
  html += "            display: flex;";
  html += "            justify-content: center;";
  html += "            align-items: center;";
  html += "        }";
  html += "        h1 {";
  html += "            text-align: center;";
  html += "            margin-top: 30px;";
  html += "            font-weight: bold;";
  html += "            color: #fdfdfd;";
  html += "        }";
  html += "        .container {";
  html += "            width: 90%;";
  html += "            max-width: 900px;";
  html += "            padding: 30px;";
  html += "            background-color: rgba(255, 255, 255, 0.1);";
  html += "            border-radius: 20px;";
  html += "            box-shadow: 0px 10px 30px rgba(0, 0, 0, 0.3);";
  html += "            backdrop-filter: blur(10px);";
  html += "        }";
  html += "        .card {";
  html += "            flex: 1;";
  html += "            background: rgba(255, 255, 255, 0.15);";
  html += "            backdrop-filter: blur(8px);";
  html += "            border: none;";
  html += "            border-radius: 15px;";
  html += "            padding: 20px;";
  html += "            text-align: center;";
  html += "        }";
  html += "        .data-display {";
  html += "            font-size: 1.8rem;";
  html += "            font-weight: bold;";
  html += "            margin-bottom: 10px;";
  html += "        }";
  html += "        .data-label {";
  html += "            font-weight: bold;";
  html += "        }";
  html += "    </style>";
  html += "</head>";
  html += "<body>";
  html += "<div class=\"container\">";
  html += "    <h1>Evaporation Meter</h1>";
  html += "    <div class=\"row\">";
  html += "        <div class=\"card\">";
  html += "            <div class=\"data-display\">";
  html += "                <span class=\"data-label\">Water Level:</span>";
  html += "                <span id=\"waterLevel\">0</span> mm";
  html += "            </div>";
  html += "        </div>";
  html += "        <div class=\"card\">";
  html += "            <div class=\"data-display\">";
  html += "                <span class=\"data-label\">Wind Speed:</span>";
  html += "                <span id=\"windSpeed\">0</span> m/s";
  html += "            </div>";
  html += "        </div>";
  html += "    </div>";
  html += "</div>";
  html += "<script>";
  html += "function fetchData(type, elementId) {";
  html += "  fetch(`/data?type=${type}`)"; // Fetch data based on type (ultra or wind)
  html += "    .then(response => response.text())";
  html += "    .then(data => { document.getElementById(elementId).innerText = data; });";
  html += "}";
  html += "setInterval(() => fetchData('ultra', 'waterLevel'), 100);";  // Fetch water level data every 100 ms
  html += "setInterval(() => fetchData('wind', 'windSpeed'), 1000);";  // Fetch wind speed data every 1 second
  html += "fetchData('ultra', 'waterLevel');";  // Initial fetch for water level
  html += "fetchData('wind', 'windSpeed');";    // Initial fetch for wind speed";
  html += "</script>";
  html += "</body></html>";

  // Send the HTML response to the client
  server.send(200, "text/html", html);
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Set up the ESP32 as an access point
  WiFi.softAP(ssid, password);

  // Print the IP address of the access point
  Serial.println("Access Point started.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define the root URL handler
  server.on("/", handleRoot);
  server.on("/data", handleData); // Combined handler for both distance and wind data

  // Start the web server
  server.begin();
  Serial.println("Web server started.");

  // Set trigPin as OUTPUT and echoPin as INPUT
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(windSpeedPin, INPUT);  // Set the pin as input
  attachInterrupt(digitalPinToInterrupt(windSpeedPin), countPulse, RISING); // Attach interrupt
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}