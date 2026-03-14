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

void readSerialCommand(){

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

}

void setup () {
    Serial.begin(115200);
    delay(1000); 
    pinMode(PIR_PIN, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);
    dht.begin();
    Wire.begin(21, 22);   // SDA = 21, SCL = 22 (ESP32)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED khong khoi dong duoc");
    while(1);
  }
  delay(2000);                 // Đợi OLED ổn định
  display.clearDisplay();         // Xóa màn hình
  display.setTextSize(1);         // Kích thước chữ
  display.setTextColor(WHITE);
}

void loop() {
    readSerialCommand();

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int adclightLevel = analogRead(LDR_PIN);
    int lightLevel = map(adclightLevel, 4095, 0, 0, 100);
    int motion = digitalRead(PIR_PIN);

    // ===== OLED DISPLAY =====
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

    // // ===== BUZZER + LED =====
    // if (motion == HIGH) {
    //     digitalWrite(BUZZER, HIGH);
    // } else {
    //     digitalWrite(BUZZER, LOW);
    // }

    // ===== SERIAL TIMER =====
    unsigned long currentTime = millis();

    // gửi TEMP HUMI LIGHT mỗi 10s
    if(currentTime - lastSendTHL >= 10000){

        Serial.print("!1:TEMP:");
        Serial.print(temperature);
        Serial.println("#");

        Serial.print("!1:HUMI:");
        Serial.print(humidity);
        Serial.println("#");

        Serial.print("!1:LIGHT:");
        Serial.print(lightLevel);
        Serial.println("#");

        lastSendTHL = currentTime;
    }

    // gửi PIR mỗi 10s
    if(currentTime - lastSendPIR >= 10000){

        Serial.print("!1:PIR:");
        Serial.print(motion == HIGH ? "True" : "False");
        Serial.println("#");

        lastSendPIR = currentTime;
    }

    delay(200);
}
