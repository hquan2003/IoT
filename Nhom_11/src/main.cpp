#include <Arduino.h>


/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6F9SbQBJO"
#define BLYNK_TEMPLATE_NAME "nhom11"
#define BLYNK_AUTH_TOKEN "nSTmDiwJPNvC4Mdvkt8GFHreksfyccPc"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

const int lightSensorPin = 34;  // Chân đọc giá trị cảm biến ánh sáng (LDR)
const int buttonPin = 23;      // Chân nút nhấn
const int ledPin = 21;          // Chân LED

bool ledState = false;        // Trạng thái đèn
bool manualMode = false;      // Chế độ điều khiển thủ công

BlynkTimer timer;

void sendToBlynk() {
    int lightValue = analogRead(lightSensorPin);
    int brightness = map(lightValue, 0, 4095, 0, 100);
    Blynk.virtualWrite(V1, ledState);
}

void setup() {
    pinMode(lightSensorPin, INPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    timer.setInterval(1000L, sendToBlynk);
}

BLYNK_WRITE(V1) {
    ledState = param.asInt();
    manualMode = true;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
}

void loop() {
    Blynk.run();
    timer.run();
    
    int lightValue = analogRead(lightSensorPin); // Đọc giá trị cảm biến ánh sáng
    int brightness = map(lightValue, 0, 4095, 0, 100); // Chuyển đổi về %
    bool buttonState = digitalRead(buttonPin) == LOW;
    
    if (buttonState) {
        delay(200); // Chống dội nút
        manualMode = !manualMode; // Chuyển chế độ thủ công
        ledState = !ledState;     // Đảo trạng thái LED
    }
    
    if (!manualMode) { // Nếu không bật chế độ thủ công
        ledState = (brightness < 25); // Bật đèn nếu ánh sáng < 25%
    }

    digitalWrite(ledPin, ledState ? HIGH : LOW); // Điều khiển đèn
    
    Serial.print("Ánh sáng: ");
    Serial.print(brightness);
    Serial.print("% - LED: ");
    Serial.println(ledState ? "BẬT" : "TẮT");
    
    delay(100);
}
