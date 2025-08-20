//====================================== BLYNK
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6g1PgR2RA"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "GC4E4o24JJcR6shnmoJbWIuUluQKSPiX"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

BlynkTimer timer;
char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PW";

//====================================== DHT11
#include "DHTesp.h"
#include <Ticker.h>
DHTesp dht;

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

TaskHandle_t tempTaskHandle = NULL;
Ticker tempTicker;
bool tasksEnabled = false;
int dhtPin = 26;

#define ledM  19
#define ledK  18
#define ledH  5
#define ledB  17
#define buzz  27

float temperature, humidity;

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
  an = map(an,0,4095,01,10);
}
void send_to_Blynk()
{ //Blynk.virtualWrite(V3,an);

  //Read DHT data and send to Blynk
  if (!tasksEnabled) 
  { tasksEnabled = true;
    if (tempTaskHandle != NULL) 
			vTaskResume(tempTaskHandle);
  }
  yield();
}

void setup()
{ Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(ledM, OUTPUT);     
  pinMode(ledK, OUTPUT);     
  pinMode(ledH, OUTPUT);    
  pinMode(ledB, OUTPUT);    
  pinMode(buzz, OUTPUT);   

  initTemp();
  tasksEnabled = true; 

  timer.setInterval(500L,kelip);
  timer.setInterval(10L,getAN);
  timer.setInterval(2000L,send_to_Blynk);
}

void loop()
{ Blynk.run();
  timer.run();
}

bool initTemp() 
{ byte resultValue = 0;
  dht.setup(dhtPin, DHTesp::DHT11);
	Serial.println("DHT initiated");

  xTaskCreatePinnedToCore(
			tempTask,                       /* Function to implement the task */
			"tempTask ",                    /* Name of the task */
			4000,                           /* Stack size in words */
			NULL,                           /* Task input parameter */
			5,                              /* Priority of the task */
			&tempTaskHandle,                /* Task handle. */
			1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) 
  { Serial.println("Failed to start task for temperature update");
    return false;
  } 
  else 
    tempTicker.attach(2, triggerGetTemp);
    // Start update of environment data every 2 seconds
  return true;
}

void triggerGetTemp() 
{ if (tempTaskHandle != NULL) 
	   xTaskResumeFromISR(tempTaskHandle);
}

void tempTask(void *pvParameters) 
{ Serial.println("tempTask loop started");
	while (1) 
  { if (tasksEnabled) 
			getTemperature();
		vTaskSuspend(NULL);
	}
}

bool getTemperature() 
{ TempAndHumidity newValues = dht.getTempAndHumidity();
  temperature = newValues.temperature;
  humidity    = newValues.humidity;

  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.print("  Humidity:");
  Serial.println(humidity);

  Blynk.virtualWrite(V3,temperature);
  Blynk.virtualWrite(V4,humidity);
  
	return true;
}
