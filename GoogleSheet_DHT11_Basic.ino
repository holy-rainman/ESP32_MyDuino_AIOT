#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>

#define WIFI_SSID "aim"
#define WIFI_PASSWORD "123456abc"

// Google Project ID
#define PROJECT_ID "aimtestuthm"

// Service Account's client email
#define CLIENT_EMAIL "aimtestuthm@aimtestuthm.iam.gserviceaccount.com"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCZZZFPY+A98g9X\n6fwR8o9IU/FGy8WGgzgYiVbxBNwg0jxDBeUXGzJOy5PMJAnXgcEINDUdsICf/yG7\nezHFMXHHCCo8hwBuyDrpqYW5c+VdFD/uu4YF7aFnCNZaj31fqxNYmNIB2e2eUM0h\nST8RfJYj2JJ6tXVbXm2YZPumOjhptl5R32F7sc0LBmA66wfRvWW35nYzzTywWOzJ\njTJHitDgFREqBq3IuYT+1+aqJoNo7eIqemsMSBUVujVUCtScTkjLHzrD75YwOBbb\nDtCHGafbKAGknbPt+Oq/qzINxQte1FAVtPKb0H4ME+xN+GqmsglPljD7yqDpy3jE\n1T23b3WLAgMBAAECggEABwWXOUMhKLTD7wpO96Na27y79rmtORMYpkbjK/VE/luh\nVpAWhJBCJUk8YsPjj3HpHC8Hn2xM9wX+zU4+YQB6bNn61Pp5Epkn2TphXLPpc/FR\nW+RJpkvSb5z/8vVOj+qM4Cles6b+MZkRSszHjk1dshZf3i4gSSZxbeBxhJJ1sqEM\nTvK9zd95Eyjr7wb4T+ehndiD3fB3CbHMAL0cSgRdNQo1BXi06g09KDB/u69hrX9V\nkkd2EwfiTZJlRkeiqpwCgdUVWlJ+eu2wHX3MD6DeBJSTNCZh0bDoT6/OcabIFnAq\nXGZJF65vNH29XTXA/692dACYHWd1OoavUpSfyQfPwQKBgQDVcK/gH3xAcMEYzN3b\nXsQSdYJHFHjtvCTSR7XdeGon0QDQ3wbJ6EZrjU/vtEUgm3lPQ4Vt4HHpTxmqrncl\npCfoTRWKr+G0s0oalD73OlrGOXWtVfqc04MkBumt9vGmLw7XI4G+tCKyXS6QJWyw\n+nYnk5xibTw5avOGNBYnNZSmlwKBgQC3++QBpitmJgwDKucAK/s+T9zesxksouUc\nVrz1/i0Du9gAJlRWVFG3W+LEFgqSLEJ6MEFq3i/T7dIiJCJHsfK9hOZCOD+loh7g\nstn00346mJnFkWcwWxSqroGBO+0wLQTkavoSoLjQNlMcay5aUWZdrxYgpc2q6QFu\ns+iHWO3bLQKBgB5Jw2t2jFdIqN4wFDuRExnbewVDLnXuAPumGXS4JbhIvGjnfTsh\nrvyMLQ/C8RxTc79/wWt6Mi/9fZLu9xFsXg1lAu2PT+QonRdweg7zRPZkKQLT9KBU\nk4OFR5vcMyZ1/xEghMTfIx1Ug8sIxIfIwwOSutpszeMH/k7JDWu6+sE5AoGBAI5w\nMxubvGm+9vnU0zZs1nxtc//uBIVsL5h7BpwkBlGnshiblBup+AHO34cT0WdNYMIc\nbekAz2Jx8oeAFhIPkHBtXZ7S0WO4WdN4ZAmJW2aeEh7E7XCgcyWZKKYW8RClKe51\na9nUGIEKLdoImMBP2MKa9LSd6NZywiskmdd/X9aZAoGBAM9M61FVUAOQ+nr44EqO\nRXkUX0iyYgNbjMsIXl+FDCeBGvZ1qTzEyBS1dCJWXhxnKrhCi1Ulq81kEh+zHpEM\nAvzwKUiYng2MzDNIGDYCm7PJYA3ol7T7Rgwl2CUTGh7gj8NbsM4GLpGbENGXoTSQ\nES/NB11SvXbfOeEWTS7IxhFn\n-----END PRIVATE KEY-----\n";

// Spreadsheet ID
const char spreadsheetId[] = "15wNl1btFKPTCAC1uRqb7UW5OrpHMW9LHgDEeQACYDXI";

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
