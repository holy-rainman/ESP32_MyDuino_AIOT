#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>

#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PW"

// Google Project ID
#define PROJECT_ID "YOUR_PROJECT_ID"

// Service Account's client email
#define CLIENT_EMAIL "YOUR_CLIENT_EMAIL"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nYOUR_PRIVATE_KEY_hFn\n-----END PRIVATE KEY-----\n";

// Spreadsheet ID
const char spreadsheetId[] = "YOUR_SHEET ID";

// Timer
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Token Callback
void tokenStatusCallback(TokenInfo info);

// DHT11
#include "DHTesp.h"
#include <Ticker.h>
DHTesp dht;
TaskHandle_t tempTaskHandle = NULL;
Ticker tempTicker;
bool tasksEnabled = false;
int dhtPin = 26;

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

float temperature, humidity;

void getDHT11() {
  if (!tasksEnabled) {
    tasksEnabled = true;
    if (tempTaskHandle != NULL) vTaskResume(tempTaskHandle);
  }
  yield();
}

bool initTemp() {
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");

  xTaskCreatePinnedToCore(tempTask, "tempTask ", 4000, NULL, 5, &tempTaskHandle, 1);
  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    tempTicker.attach(2, triggerGetTemp);
    return true;
  }
}

void triggerGetTemp() {
  if (tempTaskHandle != NULL) xTaskResumeFromISR(tempTaskHandle);
}

void tempTask(void *pvParameters) {
  Serial.println("tempTask loop started");
  while (1) {
    if (tasksEnabled) getTemperature();
    vTaskSuspend(NULL);
  }
}

bool getTemperature() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
  temperature = newValues.temperature;
  humidity    = newValues.humidity;

  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.print("  Humidity:");
  Serial.println(humidity);
  
  return true;
}

//============================== SETUP
void setup() {
  Serial.begin(115200);
  Serial.println();

  // Malaysia timezone UTC+8
  configTzTime("CST-8", "pool.ntp.org", "time.nist.gov");
  GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  GSheet.setTokenCallback(tokenStatusCallback);
  GSheet.setPrerefreshSeconds(10 * 60);
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

  initTemp();
  tasksEnabled = true;
}

//============================== LOOP
void loop() {
  bool ready = GSheet.ready();

  if (ready && millis() - lastTime > timerDelay) {
    lastTime = millis();

    FirebaseJson response;
    FirebaseJson valueRange;

    // DHT11 readings
    getDHT11();

    // Timestamp Malaysia time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeString[25];
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

      valueRange.add("majorDimension", "COLUMNS");
      valueRange.set("values/[0]/[0]", timeString);
      valueRange.set("values/[1]/[0]", temperature);
      valueRange.set("values/[2]/[0]", humidity);

      Serial.print("Timestamp: ");
      Serial.println(timeString);
    }

    bool success = GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &valueRange);
    if (success) {
      response.toString(Serial, true);
      valueRange.clear();
    } else {
      Serial.println(GSheet.errorReason());
    }

    Serial.println();
    Serial.println(ESP.getFreeHeap());
  }
}

//============================== TOKEN CALLBACK
void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", 
      GSheet.getTokenType(info).c_str(), 
      GSheet.getTokenStatus(info).c_str());
  }
}
