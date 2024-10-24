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
  windSpeed = pulseCopy * (0.01); // Replace 0.01 with your conversion factor
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
  html += "";
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
  html += "";
  html += "        h1 {";
  html += "            text-align: center;";
  html += "            margin-top: 30px;";
  html += "            font-weight: bold;";
  html += "            color: #fdfdfd;";
  html += "        }";
  html += "";
  html += "        .container {";
  html += "            width: 90%;";
  html += "            max-width: 900px;";
  html += "            padding: 30px;";
  html += "            background-color: rgba(255, 255, 255, 0.1);";
  html += "            border-radius: 20px;";
  html += "            box-shadow: 0px 10px 30px rgba(0, 0, 0, 0.3);";
  html += "            backdrop-filter: blur(10px);";
  html += "        }";
  html += "";
  html += "        .row {";
  html += "            display: flex;";
  html += "            justify-content: space-between;";
  html += "            flex-wrap: wrap;";
  html += "            gap: 20px;";
  html += "        }";
  html += "";
  html += "        .card {";
  html += "            flex: 1;";
  html += "            background: rgba(255, 255, 255, 0.15);";
  html += "            backdrop-filter: blur(8px);";
  html += "            border: none;";
  html += "            border-radius: 15px;";
  html += "            padding: 20px;";
  html += "            text-align: center;";
  html += "            transition: all 0.3s ease;";
  html += "        }";
  html += "";
  html += "        .data-display {";
  html += "            font-size: 1.8rem;";
  html += "            font-weight: bold;";
  html += "            margin-bottom: 10px;";
  html += "        }";
  html += "";
  html += "        .data-label {";
  html += "            font-weight: bold;";
  html += "        }";
  html += "";
  html += "        .icon {";
  html += "            margin-bottom: 10px;";
  html += "            font-size: 2.5rem;";
  html += "            color: #f64c72;";
  html += "        }";
  html += "";
  html += "        .btn-primary {";
  html += "            background-color: #f64c72;";
  html += "            border-color: #f64c72;";
  html += "            transition: background-color 0.3s ease, transform 0.3s ease;";
  html += "            font-size: 1.2rem;";
  html += "            padding: 10px 30px;";
  html += "            border-radius: 10px;";
  html += "            margin-top: 30px;";
  html += "            color: white;";
  html += "            cursor: pointer;";
  html += "        }";
  html += "";
  html += "        .chart-container {";
  html += "            margin-top: 50px;";
  html += "            background-color: rgba(255, 255, 255, 0.2);";
  html += "            padding: 30px;";
  html += "            border-radius: 15px;";
  html += "            box-shadow: 0px 10px 20px rgba(0, 0, 0, 0.3);";
  html += "            position: relative;";
  html += "        }";
  html += "";
  html += "        canvas {";
  html += "            width: 100%;";
  html += "            max-height: 400px;";
  html += "            background-color: #ffffff;";
  html += "            border-radius: 10px;";
  html += "        }";
  html += "";
  html += "        .legend {";
  html += "            position: absolute;";
  html += "            background: rgba(255, 255, 255, 0.8);";
  html += "            border-radius: 8px;";
  html += "            padding: 10px;";
  html += "            font-size: 14px;";
  html += "            color: #333;";
  html += "            z-index: 10;";
  html += "        }";
  html += "";
  html += "        .legend-item {";
  html += "            display: flex;";
  html += "            align-items: center;";
  html += "            margin-bottom: 5px;";
  html += "        }";
  html += "";
  html += "        .legend-color {";
  html += "            width: 15px;";
  html += "            height: 15px;";
  html += "            border-radius: 3px;";
  html += "            margin-right: 5px;";
  html += "        }";
  html += "    </style>";
  html += "</head>";
  html += "<body>";
  html += "";
  html += "<div class=\"container\">";
  html += "    <h1>Evaporation Meter</h1>";
  html += "    <div class=\"row\">";
  html += "        <div class=\"card\">";
  html += "            <div class=\"data-display\">";
  html += "                <span class=\"data-label\">Water Level:</span>";
  html += "                <span id=\"waterLevel\">0</span> mm";
  html += "            </div>";
  html += "        </div>";
  html += "";
  html += "        <div class=\"card\">";
  html += "            <div class=\"data-display\">";
  html += "                <span class=\"data-label\">Wind Speed:</span>";
  html += "                <span id=\"windSpeed\">0</span> m/s";
  html += "            </div>";
  html += "        </div>";
  html += "";
  html += "        <div class=\"card\">";
  html += "            <div class=\"data-display\">";
  html += "                <span class=\"data-label\">Evaporation Rate:</span>";
  html += "                <span id=\"evaporationRate\">0</span> mm/hr";
  html += "            </div>";
  html += "        </div>";
  html += "    </div>";
  html += "";
  html += "    <div class=\"text-center mt-4\">";
  html += "        <button class=\"btn-primary\" onclick=\"setReference()\">Set Current Level as Reference</button>";
  html += "    </div>";
  html += "";
  html += "    <div class=\"chart-container\">";
  html += "        <canvas id=\"evaporationChart\"></canvas>";
  html += "        <div class=\"legend\">";
  html += "            <div class=\"legend-item\">";
  html += "                <div class=\"legend-color\" style=\"background-color: rgba(255, 99, 132, 1);\"></div>";
  html += "                Water Level";
  html += "            </div>";
  html += "            <div class=\"legend-item\">";
  html += "                <div class=\"legend-color\" style=\"background-color: rgba(54, 162, 235, 1);\"></div>";
  html += "                Wind Speed";
  html += "            </div>";
  html += "        </div>";
  html += "    </div>";
  html += "</div>";
  html += "";
  html += "<script>";
  html += "    let currentTime = 0;";
  html += "    let evaporationData = [];";
  html += "    let waterLevelData = [];";
  html += "    let windSpeedData = [];";
  html += "    const maxDataPoints = 20;";
  html += "";
  html += "    const canvas = document.getElementById('evaporationChart');";
  html += "    const ctx = canvas.getContext('2d');";
  html += "    canvas.width = canvas.offsetWidth;";
  html += "    canvas.height = 300;";
  html += "";
  html += "    function fetchData() {";
  html += "        // Simulate fetching new data";
  html += "        const waterLevel = Math.random() * 100;";
  html += "        const windSpeed = Math.random() * 10;";
  html += "        const evaporationRate = Math.random() * 5;";
  html += "";
  html += "        document.getElementById('waterLevel').innerText = waterLevel.toFixed(2);";
  html += "        document.getElementById('windSpeed').innerText = windSpeed.toFixed(2);";
  html += "        document.getElementById('evaporationRate').innerText = evaporationRate.toFixed(2);";
  html += "";
  html += "        updateData(waterLevel, windSpeed);";
  html += "        drawChart();";
  html += "    }";
  html += "";
  html += "    function updateData(waterLevel, windSpeed) {";
  html += "        if (waterLevelData.length >= maxDataPoints) {";
  html += "            waterLevelData.shift();";
  html += "            windSpeedData.shift();";
  html += "            evaporationData.shift();";
  html += "        }";
  html += "        waterLevelData.push(waterLevel);";
  html += "        windSpeedData.push(windSpeed);";
  html += "        evaporationData.push(waterLevel * 0.05 * windSpeed); // Example evaporation calculation";
  html += "    }";
  html += "";
  html += "    function drawChart() {";
  html += "        ctx.clearRect(0, 0, canvas.width, canvas.height);";
  html += "        const gradient1 = ctx.createLinearGradient(0, 0, 0, 300);";
  html += "        gradient1.addColorStop(0, 'rgba(255, 99, 132, 1)');";
  html += "        gradient1.addColorStop(1, 'rgba(255, 99, 132, 0.2)');";
  html += "";
  html += "        const gradient2 = ctx.createLinearGradient(0, 0, 0, 300);";
  html += "        gradient2.addColorStop(0, 'rgba(54, 162, 235, 1)');";
  html += "        gradient2.addColorStop(1, 'rgba(54, 162, 235, 0.2)');";
  html += "";
  html += "        ctx.fillStyle = gradient1;";
  html += "        ctx.beginPath();";
  html += "        ctx.moveTo(0, canvas.height - waterLevelData[0]);";
  html += "        for (let i = 1; i < waterLevelData.length; i++) {";
  html += "            ctx.lineTo(i * (canvas.width / maxDataPoints), canvas.height - waterLevelData[i]);";
  html += "        }";
  html += "        ctx.lineTo(canvas.width, canvas.height);";
  html += "        ctx.lineTo(0, canvas.height);";
  html += "        ctx.fill();";
  html += "";
  html += "        ctx.fillStyle = gradient2;";
  html += "        ctx.beginPath();";
  html += "        ctx.moveTo(0, canvas.height - windSpeedData[0]);";
  html += "        for (let i = 1; i < windSpeedData.length; i++) {";
  html += "            ctx.lineTo(i * (canvas.width / maxDataPoints), canvas.height - windSpeedData[i]);";
  html += "        }";
  html += "        ctx.lineTo(canvas.width, canvas.height);";
  html += "        ctx.lineTo(0, canvas.height);";
  html += "        ctx.fill();";
  html += "    }";
  html += "";
  html += "    function setReference() {";
  html += "        // Set the current water level as reference";
  html += "        alert('Reference water level set to: ' + document.getElementById('waterLevel').innerText + ' mm');";
  html += "    }";
  html += "";
  html += "    setInterval(fetchData, 2000);"; // Fetch new data every 2 seconds
  html += "</script>";
  html += "";
  html += "</body>";
  html += "</html>";

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