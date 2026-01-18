#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PW";

BlynkTimer timer;

#define ledM  19
#define ledK  18
#define ledH  5
#define ledB  17
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
int status=0;
void kelip()
{ digitalWrite(ledB,status);
  status=!status;
}

int an;
void getAN()
{ an = analogRead(34);
  an = map(an,0,4095,1,10);
  Serial.println(an);
}
void send_to_Blynk()
{ Blynk.virtualWrite(V3,an);
}

void setup()
{ Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(ledM, OUTPUT);     
  pinMode(ledK, OUTPUT);     
  pinMode(ledH, OUTPUT);    
  pinMode(ledB, OUTPUT);    
  pinMode(buzz, OUTPUT);    
  timer.setInterval(500L,kelip);
  timer.setInterval(10L,getAN);
  timer.setInterval(2000L,send_to_Blynk);
}

void loop()
{ Blynk.run();
  timer.run();
}
