#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6g1PgR2RA"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "GC4E4o24JJcR6shnmoJbWIuUluQKSPiX"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PW";

#define ledM  19
#define ledK  18
#define ledH  5
#define buzz  27

BLYNK_WRITE(V0)
{ int pbM_Status = param.asInt();
  digitalWrite(ledM,pbM_Status);
  beep(1,50);
}
BLYNK_WRITE(V1)
{ int pbK_Status = param.asInt();
  digitalWrite(ledK,pbK_Status);
  beep(1,50);
}
BLYNK_WRITE(V2)
{ int pbH_Status = param.asInt();
  digitalWrite(ledH,pbH_Status);
  beep(1,50);
}
void beep(int bil, int tempoh)
{ for(int i=0;i<bil;i++)
  { digitalWrite(buzz,HIGH);  delay(tempoh);
    digitalWrite(buzz,LOW);   delay(tempoh);
  }
}

void setup()
{ Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(ledM, OUTPUT);     
  pinMode(ledK, OUTPUT);     
  pinMode(ledH, OUTPUT);    
  pinMode(buzz, OUTPUT);    
}

void loop()
{ Blynk.run();
}

