#define BLYNK_TEMPLATE_ID "TMPLXXXXXX"       
#define BLYNK_TEMPLATE_NAME "RoadPulse"
#define BLYNK_AUTH_TOKEN "k53JqlD8pCz-0Z_ixoqYMSyMgkMVN-pw"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <MPU6050.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "test1234";          
char pass[] = "qwertyuiop";      

MPU6050 mpu;

#define I2C_SDA 21
#define I2C_SCL 22

// Thresholds
const float MINOR_THRESHOLD    = 1.50;
const float MODERATE_THRESHOLD = 2.00;
const float SEVERE_THRESHOLD   = 2.60;

const unsigned long EVENT_COOLDOWN_MS = 1500;
unsigned long lastEventTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("       ROADPULSE TELEMETRY NODE          ");
  Serial.println("=========================================");

  Wire.begin(I2C_SDA, I2C_SCL);
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("[SYSTEM] MPU6050 initialized successfully.");
  } else {
    Serial.println("[SYSTEM] MPU6050 connection failed! Check I2C bus.");
  }

  Serial.println("[SYSTEM] Connecting to network and Blynk Cloud...");
  Blynk.begin(auth, ssid, pass, "blr1.blynk.cloud", 8080);
  Serial.println("[SYSTEM] Telemetry link online.");
}

void loop() {
  Blynk.run();

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float gX = (float)ax / 16384.0;
  float gY = (float)ay / 16384.0;
  float gZ = (float)az / 16384.0;

  float totalG = sqrt(gX * gX + gY * gY + gZ * gZ);

  if (millis() - lastEventTime > EVENT_COOLDOWN_MS) {
    if (totalG >= MINOR_THRESHOLD) {
      lastEventTime = millis();

      String severity = "YELLOW";
      if (totalG >= SEVERE_THRESHOLD) {
        severity = "RED";
      } else if (totalG >= MODERATE_THRESHOLD) {
        severity = "ORANGE";
      }

      String anomalyType = "POTHOLE";
      if (abs(gZ) > 1.8 && abs(gX) < 0.6) {
        anomalyType = "SPEEDBREAKER";
      }

      String payload = anomalyType + "," + severity + "," + String(totalG, 2);

      // Exact output format as shown in terminal logs:
      // [TELEMETRY] Event: POTHOLE | Severity: RED | Peak Acceleration: 10.24 g | Payload: POTHOLE,RED,10.24
      Serial.print("[TELEMETRY] Event: ");
      Serial.print(anomalyType);
      Serial.print(" | Severity: ");
      Serial.print(severity);
      Serial.print(" | Peak Acceleration: ");
      Serial.print(totalG, 2);
      Serial.print(" g | Payload: ");
      Serial.println(payload);

      Blynk.virtualWrite(V1, payload);
    }
  }

  delay(20);
}