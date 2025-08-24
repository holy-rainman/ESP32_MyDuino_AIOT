//====================================== LIBRARIES
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>

//====================================== WIFI
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PW"

//====================================== GOOGLE SHEET
#define PROJECT_ID "YOUR_PROJECT_ID"
#define CLIENT_EMAIL "YOUR_CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\n_YOUR_PRIVATE_KEY_n\n-----END PRIVATE KEY-----\n";
const char spreadsheetId[] = "YOUR_SHEET_ID";

// Token callback
void tokenStatusCallback(TokenInfo info);

// Timer untuk Google Sheet
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

//====================================== BLYNK
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID   "TMPL6g1PgR2RA"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN    "GC4E4o24JJcR6shnmoJbWIuUluQKSPiX"

#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

BlynkTimer timer;

//====================================== DHT11
#include "DHTesp.h"
#include <Ticker.h>
DHTesp dht;
TaskHandle_t tempTaskHandle = NULL;
Ticker tempTicker;
bool tasksEnabled = false;
int dhtPin = 26;

float temperature, humidity;

//====================================== LED + BUZZER
#define ledM  19
#define ledK  18
#define ledH  5
#define ledB  17
#define buzz  27

//====================================== BLYNK FUNCTIONS
BLYNK_WRITE(V0) {
  int pbM_Status = param.asInt();
  digitalWrite(ledM,pbM_Status);
  beep(1,50);
}
BLYNK_WRITE(V1) {
  int pbK_Status = param.asInt();
  digitalWrite(ledK,pbK_Status);
  beep(1,50);
}
BLYNK_WRITE(V2) {
  int pbH_Status = param.asInt();
  digitalWrite(ledH,pbH_Status);
  beep(1,50);
}
void beep(int bil, int tempoh) {
  for(int i=0;i<bil;i++) {
    digitalWrite(buzz,HIGH);  delay(tempoh);
    digitalWrite(buzz,LOW);   delay(tempoh);
  }
}
int status=0;
void kelip() {
  digitalWrite(ledB,status);
  status=!status;
}

//====================================== ANALOG
int an;
void getAN() {
  an = analogRead(34);
  an = map(an,0,4095,01,10);
}

//====================================== DHT TASK
void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

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

  // Hantar ke Blynk
  Blynk.virtualWrite(V3,temperature);
  Blynk.virtualWrite(V4,humidity);

  return true;
}

//====================================== BLYNK TASK
void send_to_Blynk() {
  if (!tasksEnabled) {
    tasksEnabled = true;
    if (tempTaskHandle != NULL) vTaskResume(tempTaskHandle);
  }
  yield();
}

//====================================== SETUP
void setup() {
  Serial.begin(115200);
  Serial.println();

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

  // Blynk init
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  // Malaysia timezone UTC+8
  configTzTime("MYT-8", "pool.ntp.org", "time.nist.gov");
  GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

  // TUNGGU NTP SYNC SIAP
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP...");
    delay(1000);
  }
  Serial.println(&timeinfo, "Local time (MYT): %Y-%m-%d %H:%M:%S");

  pinMode(ledM, OUTPUT);     
  pinMode(ledK, OUTPUT);     
  pinMode(ledH, OUTPUT);    
  pinMode(ledB, OUTPUT);    
  pinMode(buzz, OUTPUT);   

  // Google Sheets
  GSheet.setTokenCallback(tokenStatusCallback);
  GSheet.setPrerefreshSeconds(10 * 60);
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

  // DHT11
  initTemp();
  tasksEnabled = true; 

  // Timer
  timer.setInterval(500L,kelip);
  timer.setInterval(10L,getAN);
  timer.setInterval(2000L,send_to_Blynk);
}

//====================================== LOOP
void loop() {
  Blynk.run();
  timer.run();

  // Hantar ke Google Sheets
  if (GSheet.ready() && millis() - lastTime > timerDelay) {
    lastTime = millis();

    FirebaseJson response;
    FirebaseJson valueRange;

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
  }
}

//====================================== TOKEN CALLBACK
void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", 
      GSheet.getTokenType(info).c_str(), 
      GSheet.getTokenStatus(info).c_str());
  }
}
