#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

BlynkTimer timer;

char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PW";

int status=0;
void kelip()
{ digitalWrite(17,status);
  status=!status;
}

void setup()
{ pinMode(17,OUTPUT);
  
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(500L,kelip);
}

void loop()
{ Blynk.run();
  timer.run();
}
