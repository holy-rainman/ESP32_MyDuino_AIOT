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
  float temperature = newValues.temperature;
  float humidity    = newValues.humidity;

  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.print("  Humidity:");
  Serial.println(humidity);
  
	return true;
}

void setup()
{ Serial.begin(115200);
  Serial.println();
  Serial.println("DHT ESP32 example with tasks");
  initTemp();
  tasksEnabled = true;
}

void loop() 
{ if (!tasksEnabled) 
  { // Wait 2 seconds to let system settle down
    delay(2000);
    tasksEnabled = true;
    if (tempTaskHandle != NULL) 
			vTaskResume(tempTaskHandle);
  }
  yield();
}
