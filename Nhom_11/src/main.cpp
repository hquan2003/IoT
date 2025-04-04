#include "credentials.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#define BLYNK_TEMPLATE_ID "TMPL65DK4CDng"
#define BLYNK_TEMPLATE_NAME "IoT"
#define BLYNK_AUTH_TOKEN "NLYspzTfj4TOVBQAiy1nWnHzQsNPk-ev"

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

const int lightSensorPin = 34;
const int buttonPin = 23;
const int ledPin = 21;

bool trangthai_led = false;
bool chedo_led = false;

int offHour = 0, offMinute = 0;

BlynkTimer timer; // Tạo bộ hẹn giờ trong Blynk
WidgetRTC rtc; // Tạo đồng hồ thời gian thực

void sendToBlynk() {
    int giatri_as = analogRead(lightSensorPin);
    int dosang = map(giatri_as, 0, 4095, 0, 100);
    Blynk.virtualWrite(V1, trangthai_led);
}

void setup() {
    pinMode(lightSensorPin, INPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    rtc.begin(); // Khởi động đồng hồ thời gian thực
    timer.setInterval(1000L, sendToBlynk);
}

BLYNK_WRITE(V1) {
    trangthai_led = param.asInt();
    chedo_led = true;
    digitalWrite(ledPin, trangthai_led ? HIGH : LOW);
    Serial.println(trangthai_led ? "LED BẬT (điều khiển từ Blynk)" : "LED TẮT (điều khiển từ Blynk)");
}

BLYNK_WRITE(V2) { 
    TimeInputParam t(param);

    if (t.hasStartTime()) { // Kiểm tra nếu có thời gian hợp lệ
        offHour = t.getStartHour();
        offMinute = t.getStartMinute();

        Serial.print("Giờ tắt đèn cập nhật: ");
        Serial.print(offHour);
        Serial.print(":");
        Serial.println(offMinute);
    }
}

void loop() {
    Blynk.run();
    timer.run();
    
    int giatri_as = analogRead(lightSensorPin);
    int dosang = (giatri_as * 100) / 4095;
    bool buttonState = digitalRead(buttonPin) == LOW;
    
    if (buttonState) {
        delay(200);
        chedo_led = !chedo_led;
        trangthai_led = !trangthai_led;
        Serial.println(trangthai_led ? "LED BẬT (điều khiển bằng nút nhấn)" : "LED TẮT (điều khiển bằng nút nhấn)");
    }
    
    if (!chedo_led) {
        trangthai_led = (dosang < 25);
    }

    // Kiểm tra nếu đến giờ tắt đèn (dựa vào RTC)
    if (hour() == offHour && minute() == offMinute) {
        trangthai_led = false;
        chedo_led = false; 
    }
    
    digitalWrite(ledPin, trangthai_led ? HIGH : LOW);

    // Cập nhật log Serial nếu có thay đổi độ sáng
    static int tam = -1;

    if (dosang != tam) {
        tam = dosang;
        Serial.print("Ánh sáng: ");
        Serial.print(dosang);
        Serial.print("% - LED: ");
        Serial.println(trangthai_led ? "BẬT" : "TẮT");
    }
    
    delay(100);
}
