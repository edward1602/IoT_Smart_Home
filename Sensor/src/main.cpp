#include <Arduino.h>
#include "DHT.h"
#include <Wire.h>                // Thư viện I2C
#include <Adafruit_GFX.h>        // Thư viện đồ họa Adafruit
#include <Adafruit_SSD1306.h>    // Thư viện OLED SSD1306
#include "freertos/semphr.h"

#define DHT_PIN 23
#define SCREEN_WIDTH 128   // Chiều rộng OLED (pixel)
#define SCREEN_HEIGHT 64   // Chiều cao OLED (pixel)
#define LDR_PIN 34
#define OLED_RESET -1
#define PIR_PIN 27
#define BUZZER 25
#define LED 2
#define FAN 4

float humidity = NAN;
float temperature = NAN;
int adclightLevel;
int lightLevel;
int motion;

DHT dht(DHT_PIN, DHT11);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SemaphoreHandle_t dataMutex;

String serialBuffer = "";

void handleCommand(String cmd){

    if(cmd == "0"){          // bật LED
        digitalWrite(LED, HIGH);
    }

    else if(cmd == "1"){     // tắt LED
        digitalWrite(LED, LOW);
    }

    else if(cmd == "2"){     // bật buzzer
        digitalWrite(BUZZER, HIGH);
    }

    else if(cmd == "3"){     // tắt buzzer
        digitalWrite(BUZZER, LOW);
    }
    else if(cmd == "4"){     // bật quạt
        digitalWrite(FAN, HIGH);
    }

    else if(cmd == "5"){     // tắt quạt
        digitalWrite(FAN, LOW);
    }

}

void Task_Read_DTH_Sensor(void *pvParametters){
  while(1){
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();

  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  }
  if (!isnan(newTemperature)) {
    temperature = newTemperature;
  }
  xSemaphoreGive(dataMutex);
  }
  vTaskDelay(5000);
  }
}

void Task_Read_LDR_Sensor(void *pvParametters){
  while(1){
  adclightLevel = analogRead(LDR_PIN);
  lightLevel = map(adclightLevel, 4095, 0, 0, 100);
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    lightLevel = map(adclightLevel, 4095, 0, 0, 100);
    xSemaphoreGive(dataMutex);
  }
  vTaskDelay(5000);
  }
}

void Task_Read_PIR_Sensor(void *pvParemetters){
  while(1){
  int newMotion = digitalRead(PIR_PIN);
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    motion = newMotion;
    xSemaphoreGive(dataMutex);
  }
  vTaskDelay(1000);
  }
}

void Task_Display_OLED(void *pvParametters){
  while(1){
  float localTemperature;
  float localHumidity;
  int localLightLevel;
  int localMotion;
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    localTemperature = temperature;
    localHumidity = humidity;
    localLightLevel = lightLevel;
    localMotion = motion;
    xSemaphoreGive(dataMutex);
  }

  display.clearDisplay();

  display.setCursor(0,10);
  display.print("Temperature: ");
  display.print(localTemperature);
  display.println(" C");

  display.setCursor(0,25);
  display.print("Humidity: ");
  display.print(localHumidity);
  display.println(" %");

  display.setCursor(0,40);
  display.print("Light: ");
  display.print(localLightLevel);
  display.println(" lux");

  display.setCursor(0,55);
  display.print("Motion: ");
  display.println(localMotion == HIGH ? 1 : 0);

  display.display();
  vTaskDelay(2000);
  }
}

void Task_Print_DTH11(void* pvParametters){
  float lastHumidity = NAN;
  float lastTemperature = NAN;

  while(1){
    float localTemperature;
    float localHumidity;
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      localTemperature = temperature;
      localHumidity = humidity;
      xSemaphoreGive(dataMutex);
    }

    if (localTemperature != lastTemperature) {
      Serial.print("!1:TEMP:");
      Serial.print(localTemperature);
      Serial.println("#");
      lastTemperature = localTemperature;
    }
    if (localHumidity != lastHumidity) {
      Serial.print("!1:HUMI:");
      Serial.print(localHumidity);
      Serial.println("#");
      lastHumidity = localHumidity;
    }
    vTaskDelay(15000);
  }
}

void Task_Print_Light(void* pvParametters){
  int lastLightLevel = -1;

  while(1){
    int localLightLevel;
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      localLightLevel = lightLevel;
      xSemaphoreGive(dataMutex);
    }

    if (localLightLevel != lastLightLevel) {
      Serial.print("!1:LIGHT:");
      Serial.print(localLightLevel);
      Serial.println("#");
      lastLightLevel = localLightLevel;
    }
    vTaskDelay(8000);
  }
}

void Task_Print_Motion(void* pvParametters){
  int lastMotion = -1;
  unsigned long lastReportMs = 0;

  while(1){
    unsigned long now = millis();
    int localMotion;
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      localMotion = motion;
      xSemaphoreGive(dataMutex);
    }

    if (localMotion != lastMotion && (now - lastReportMs) >= 2000) {
      Serial.print("!1:PIR:");
      Serial.print(localMotion == HIGH ? "True" : "False");
      Serial.println("#");
      lastMotion = localMotion;
      lastReportMs = now;
    }
    vTaskDelay(200);
  }
}

void Task_Read_Serial_Command(void *pvParametters){
  while(1){
     while(Serial.available()){
        char c = Serial.read();

        if(c == '#'){               
            handleCommand(serialBuffer);
            serialBuffer = "";
        }
        else{
            serialBuffer += c;

            if(serialBuffer.length() > 10){ // chống tràn
                serialBuffer = "";
            }
        }
    }
  vTaskDelay(100);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); 

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(FAN, OUTPUT);

  dht.begin();
  Wire.begin(21, 22);   

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
  Serial.println("OLED khong khoi dong duoc");
  while(1);
  }
  delay(2000);                 
  display.clearDisplay();      
  display.setTextSize(1);       
  display.setTextColor(WHITE);

  dataMutex = xSemaphoreCreateMutex();
  if (dataMutex == NULL) {
    Serial.println("Mutex khong khoi dong duoc");
    while(1);
  }

  xTaskCreate(Task_Read_DTH_Sensor, "DTH sensor", 2048, NULL, 2, NULL);
  xTaskCreate(Task_Read_LDR_Sensor, "LDR sensor", 2048, NULL, 2, NULL);
  xTaskCreate(Task_Read_PIR_Sensor, "PIR sensor", 2048, NULL, 2, NULL);
  xTaskCreate(Task_Print_DTH11, "Serial Print", 2048, NULL, 1, NULL);
  xTaskCreate(Task_Print_Motion, "Serial Print Motion", 2048, NULL, 1, NULL);
  xTaskCreate(Task_Print_Light, "Serial Print Light", 2048, NULL, 1, NULL);
  xTaskCreate(Task_Read_Serial_Command, "Serial Read", 2048, NULL, 1, NULL);
  xTaskCreate(Task_Display_OLED, "Dislay OLED", 2048, NULL, 3, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}

