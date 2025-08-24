#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_FIRMWARE_VERSION  "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_ESP32_DEV_MODULE
#include <WiFi.h>
#include "BlynkEdgent.h"
BlynkTimer timer;

bool status=0;
void kelip()
{ digitalWrite(19,status);
  status=!status;
}
void setup()
{ Serial.begin(9600);
  BlynkEdgent.begin();
  timer.setInterval(1000L,kelip);
  pinMode(19,OUTPUT);
}

void loop() 
{ BlynkEdgent.run();
  timer.run();
}
