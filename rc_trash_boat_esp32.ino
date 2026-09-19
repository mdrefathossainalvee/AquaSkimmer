/*
  RC Trash Sweeping Boat
  Controller: ESP32 + 2 ESCs + 2 DC motors
  Control: Wi-Fi Access Point + Web Browser

  IMPORTANT:
  - This sketch assumes bidirectional ESCs with a neutral pulse near 1500 us.
  - If your ESCs are forward-only, adapt the pulse limits and mixing logic.
  - Test with propellers removed and boat secured.
*/

#include <WiFi.h>
#include <WebServer.h>

// ---------- Wi-Fi ----------
const char* AP_SSID = "TrashBoat-ESP32";
const char* AP_PASSWORD = "12345678";

WebServer server(80);

// ---------- ESC pins ----------
const int ESC_LEFT_PIN  = 25;
const int ESC_RIGHT_PIN = 26;

// ---------- PWM ----------
const int PWM_FREQUENCY = 50;       // ESC servo signal frequency
const int PWM_RESOLUTION = 16;      // 16-bit LEDC
const int PWM_PERIOD_US = 20000;    // 20 ms at 50 Hz

// Bidirectional ESC pulse range
const int ESC_MIN_US = 1000;
const int ESC_NEUTRAL_US = 1500;
const int ESC_MAX_US = 2000;

// Safety timeout
const unsigned long COMMAND_TIMEOUT_MS = 1000;
unsigned long lastCommandTime = 0;

// Current command values: -100 to +100
int throttleCommand = 0;
int steeringCommand = 0;

unsigned long microsecondsToDuty(int pulseUs) {
  pulseUs = constrain(pulseUs, ESC_MIN_US, ESC_MAX_US);
  return (unsigned long)((pulseUs * 65535UL) / PWM_PERIOD_US);
}

void writeEscPulse(int pin, int pulseUs) {
  ledcWrite(pin, microsecondsToDuty(pulseUs));
}

void setMotorPercent(int pin, int percent) {
  percent = constrain(percent, -100, 100);

  int pulseUs = ESC_NEUTRAL_US;

  if (percent > 0) {
    pulseUs = map(percent, 0, 100, ESC_NEUTRAL_US, ESC_MAX_US);
  } else if (percent < 0) {
    pulseUs = map(percent, -100, 0, ESC_MIN_US, ESC_NEUTRAL_US);
  }

  writeEscPulse(pin, pulseUs);
}

void stopMotors() {
  throttleCommand = 0;
  steeringCommand = 0;
  setMotorPercent(ESC_LEFT_PIN, 0);
  setMotorPercent(ESC_RIGHT_PIN, 0);
}

void applyMovement() {
  // Differential thrust mixing
  int leftMotor  = throttleCommand + steeringCommand;
  int rightMotor = throttleCommand - steeringCommand;

  leftMotor = constrain(leftMotor, -100, 100);
  rightMotor = constrain(rightMotor, -100, 100);

  setMotorPercent(ESC_LEFT_PIN, leftMotor);
  setMotorPercent(ESC_RIGHT_PIN, rightMotor);
}

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Trash Boat Controller</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #101820;
      color: #ffffff;
      text-align: center;
      margin: 0;
      padding: 18px;
    }
    .card {
      max-width: 520px;
      margin: auto;
      background: #1c2a35;
      padding: 20px;
      border-radius: 18px;
      box-shadow: 0 0 20px rgba(0,0,0,.25);
    }
    h1 { font-size: 24px; margin-top: 0; }
    .value { font-size: 28px; font-weight: bold; margin: 8px; }
    input[type=range] { width: 100%; }
    button {
      border: none;
      border-radius: 12px;
      padding: 16px;
      margin: 6px;
      font-size: 18px;
      font-weight: bold;
      cursor: pointer;
    }
    .stop { background: #e53935; color: white; width: 90%; }
    .reset { background: #607d8b; color: white; }
    .hint { color: #b9c7d0; font-size: 13px; }
  </style>
</head>
<body>
  <div class="card">
    <h1>RC Trash Sweeping Boat</h1>
    <p class="hint">Connect to the boat Wi-Fi and control both motors.</p>

    <h3>Throttle</h3>
    <div class="value"><span id="throttleValue">0</span>%</div>
    <input id="throttle" type="range" min="-100" max="100" value="0">

    <h3>Steering</h3>
    <div class="value"><span id="steeringValue">0</span>%</div>
    <input id="steering" type="range" min="-100" max="100" value="0">

    <br><br>
    <button class="stop" onclick="stopBoat()">EMERGENCY STOP</button>
    <br>
    <button class="reset" onclick="resetControls()">Center Controls</button>

    <p class="hint">Negative throttle = reverse, positive throttle = forward.</p>
  </div>

<script>
  const throttle = document.getElementById('throttle');
  const steering = document.getElementById('steering');
  const throttleValue = document.getElementById('throttleValue');
  const steeringValue = document.getElementById('steeringValue');

  let timer;

  function sendCommand() {
    throttleValue.textContent = throttle.value;
    steeringValue.textContent = steering.value;

    fetch(`/cmd?throttle=${throttle.value}&steering=${steering.value}`)
      .catch(() => {});
  }

  function delayedSend() {
    clearTimeout(timer);
    timer = setTimeout(sendCommand, 30);
  }

  throttle.addEventListener('input', delayedSend);
  steering.addEventListener('input', delayedSend);

  function stopBoat() {
    throttle.value = 0;
    steering.value = 0;
    sendCommand();
  }

  function resetControls() {
    stopBoat();
  }

  // Keep sending the command so the ESP32 safety timeout is satisfied.
  setInterval(sendCommand, 300);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", MAIN_PAGE);
}

void handleCommand() {
  if (server.hasArg("throttle")) {
    throttleCommand = constrain(server.arg("throttle").toInt(), -100, 100);
  }

  if (server.hasArg("steering")) {
    steeringCommand = constrain(server.arg("steering").toInt(), -100, 100);
  }

  lastCommandTime = millis();
  applyMovement();

  String response = "Throttle: " + String(throttleCommand) +
                    " | Steering: " + String(steeringCommand);
  server.send(200, "text/plain", response);
}

void setup() {
  Serial.begin(115200);

  // ESP32 Arduino Core 3.x LEDC API
  ledcAttach(ESC_LEFT_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(ESC_RIGHT_PIN, PWM_FREQUENCY, PWM_RESOLUTION);

  stopMotors();
  delay(3000); // Give ESCs time to initialize at neutral

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.println();
  Serial.println("Trash Boat Controller Started");
  Serial.print("Connect to Wi-Fi: ");
  Serial.println(AP_SSID);
  Serial.print("Open this address: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCommand);
  server.begin();

  lastCommandTime = millis();
}

void loop() {
  server.handleClient();

  if (millis() - lastCommandTime > COMMAND_TIMEOUT_MS) {
    stopMotors();
  }

  delay(5);
}
