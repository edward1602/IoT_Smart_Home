#include <Arduino.h>
#include "DHT.h"
#include <Wire.h>                // Thư viện I2C
#include <Adafruit_GFX.h>        // Thư viện đồ họa Adafruit
#include <Adafruit_SSD1306.h>    // Thư viện OLED SSD1306

#define DHT_PIN 23
#define SCREEN_WIDTH 128   // Chiều rộng OLED (pixel)
#define SCREEN_HEIGHT 64   // Chiều cao OLED (pixel)
#define LDR_PIN 34
#define OLED_RESET -1
#define PIR_PIN 27
#define BUZZER 25
#define LED 2

float humidity;
float temperature;
int adclightLevel;
int lightLevel;
int motion;

DHT dht(DHT_PIN, DHT11);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastSendTHL = 0;
unsigned long lastSendPIR = 0;
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

}

void Task_FAN_Control(void *pvParametters){

  while(1){

    vTaskDelay(2000);
  }
}

void Task_Read_DTH_Sensor(void *pvParametters){
  while(1){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  vTaskDelay(5000);
  }
}

void Task_Read_LDR_Sensor(void *pvParametters){
  while(1){
  adclightLevel = analogRead(LDR_PIN);
  lightLevel = map(adclightLevel, 4095, 0, 0, 100);
  vTaskDelay(5000);
  }
}

void Task_Read_PIR_Sensor(void *pvParemetters){
  while(1){
  motion = digitalRead(PIR_PIN);
  vTaskDelay(1000);
  }
}

void Task_Display_OLED(void *pvParametters){
  while(1){
  display.clearDisplay();

  display.setCursor(0,10);
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" C");

  display.setCursor(0,25);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");

  display.setCursor(0,40);
  display.print("Light: ");
  display.print(lightLevel);
  display.println(" lux");

  display.setCursor(0,55);
  display.print("Motion: ");
  display.println(motion == HIGH ? 1 : 0);

  display.display();
  vTaskDelay(2000);
  }
}

void Task_Print_Serial(void* pvParametters){
  float lastHumidity = NAN;
  float lastTemperature = NAN;
  int lastLightLevel = -1;
  int lastMotion = -1;

  while(1){
    if (temperature != lastTemperature) {
      Serial.print("!1:TEMP:");
      Serial.print(temperature);
      Serial.println("#");
      lastTemperature = temperature;
    }
    if (humidity != lastHumidity) {
      Serial.print("!1:HUMI:");
      Serial.print(humidity);
      Serial.println("#");
      lastHumidity = humidity;
    }
    if (lightLevel != lastLightLevel) {
      Serial.print("!1:LIGHT:");
      Serial.print(lightLevel);
      Serial.println("#");
      lastLightLevel = lightLevel;
    }
    if (motion != lastMotion) {
      Serial.print("!1:PIR:");
      Serial.print(motion == HIGH ? "True" : "False");
      Serial.println("#");
      lastMotion = motion;
    }
    vTaskDelay(100);
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

  xTaskCreate(Task_FAN_Control, "FAN control", 2048, NULL, 1, NULL);
  xTaskCreate(Task_Read_DTH_Sensor, "DTH sensor", 2048, NULL, 2, NULL);
  xTaskCreate(Task_Read_LDR_Sensor, "LDR sensor", 2048, NULL, 2, NULL);
  xTaskCreate(Task_Read_PIR_Sensor, "PIR sensor", 2028, NULL, 2, NULL);
  xTaskCreate(Task_Print_Serial, "Serial Print", 2028, NULL, 1, NULL);
  xTaskCreate(Task_Read_Serial_Command, "Serial Read", 2028, NULL, 1, NULL);
  xTaskCreate(Task_Display_OLED, "Dislay OLED", 2048, NULL, 3, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}

