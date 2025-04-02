#include "credentials.h" // Chứa thông tin BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME, BLYNK_AUTH_TOKEN
#include <Arduino.h> // Thư viện cơ bản cho Arduino
#include <WiFi.h> // Thư viện WiFi cho ESP32
#include <WiFiClient.h> // Hỗ trợ giao tiếp WiFi client
#include <BlynkSimpleEsp32.h> // Thư viện Blynk cho ESP32
#include <TimeLib.h> // Thư viện hỗ trợ xử lý thời gian
#include <WidgetRTC.h> // Thư viện đồng hồ thời gian thực (RTC)

#define BLYNK_TEMPLATE_ID "TMPL65DK4CDng"
#define BLYNK_TEMPLATE_NAME "IoT"
#define BLYNK_AUTH_TOKEN "NLYspzTfj4TOVBQAiy1nWnHzQsNPk-ev"

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

const int lightSensorPin = 34; // Chân đọc cảm biến ánh sáng
const int buttonPin = 23; // Chân đọc nút nhấn
const int ledPin = 21; // Chân điều khiển đèn LED

bool ledState = false; // Trạng thái LED (ban đầu là tắt)
bool manualMode = false; // Trạng thái điều khiển bằng tay (ban đầu là tự động)

int offHour = 0, offMinute = 0; // Giờ và phút tắt đèn

BlynkTimer timer; // Tạo bộ hẹn giờ trong Blynk
WidgetRTC rtc; // Tạo đồng hồ thời gian thực

// Gửi dữ liệu trạng thái LED lên Blynk mỗi giây
void sendToBlynk() {
    int lightValue = analogRead(lightSensorPin); // Đọc giá trị cảm biến ánh sáng
    int brightness = map(lightValue, 0, 4095, 0, 100); // Chuyển đổi thành phần trăm
    Blynk.virtualWrite(V1, ledState); // Gửi trạng thái LED lên Blynk
}

// Cấu hình ban đầu
void setup() {
    pinMode(lightSensorPin, INPUT); // Cảm biến ánh sáng là đầu vào
    pinMode(buttonPin, INPUT_PULLUP); // Nút nhấn có điện trở kéo lên (PULLUP)
    pinMode(ledPin, OUTPUT); // LED là đầu ra
    Serial.begin(115200); // Khởi động Serial Monitor ở tốc độ 115200 baud
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối Blynk và WiFi
    rtc.begin(); // Khởi động đồng hồ thời gian thực
    timer.setInterval(1000L, sendToBlynk); // Cứ mỗi giây gửi dữ liệu lên Blynk
}

// Nhận tín hiệu bật/tắt LED từ Blynk (V1)
BLYNK_WRITE(V1) {
    ledState = param.asInt(); // Lấy giá trị từ app Blynk
    manualMode = true; // Khi điều khiển từ Blynk, chuyển sang chế độ điều khiển tay
    digitalWrite(ledPin, ledState ? HIGH : LOW); // Bật hoặc tắt LED tương ứng
    Serial.println(ledState ? "LED BẬT (điều khiển từ Blynk)" : "LED TẮT (điều khiển từ Blynk)");
}

// Nhận thời gian tắt đèn từ Time Input Widget (V2)
BLYNK_WRITE(V2) { 
    TimeInputParam t(param);

    if (t.hasStartTime()) { // Kiểm tra nếu có thời gian hợp lệ
        offHour = t.getStartHour(); // Lấy giờ từ widget Blynk
        offMinute = t.getStartMinute(); // Lấy phút từ widget Blynk

        Serial.print("Giờ tắt đèn cập nhật: ");
        Serial.print(offHour);
        Serial.print(":");
        Serial.println(offMinute);
    }
}

// Vòng lặp chính
void loop() {
    Blynk.run(); // Chạy Blynk
    timer.run(); // Chạy bộ hẹn giờ để gửi dữ liệu
    
    int lightValue = analogRead(lightSensorPin); // Đọc giá trị cảm biến ánh sáng
    int brightness = (lightValue * 100) / 4095; // Chuyển đổi về phần trăm ánh sáng
    bool buttonState = digitalRead(buttonPin) == LOW; // Kiểm tra nút có nhấn không
    
    if (buttonState) { // Nếu nút được nhấn
        delay(200); // Chống dội nút
        manualMode = !manualMode; // Chuyển chế độ (tự động ↔️ thủ công)
        ledState = !ledState; // Đảo trạng thái LED
        Serial.println(ledState ? "LED BẬT (điều khiển bằng nút nhấn)" : "LED TẮT (điều khiển bằng nút nhấn)");
    }
    
    if (!manualMode) { // Chế độ tự động
        ledState = (brightness < 25); // Nếu ánh sáng < 25% thì bật đèn
    }

    // Kiểm tra nếu đến giờ tắt đèn (dựa vào RTC)
    if (hour() == offHour && minute() == offMinute) {
        ledState = false; // Tắt đèn 
    }
    
    digitalWrite(ledPin, ledState ? HIGH : LOW); // Bật/tắt LED theo trạng thái

    // Cập nhật log Serial nếu có thay đổi độ sáng
    static int lastBrightness = -1;

    if (brightness != lastBrightness) {
        lastBrightness = brightness;
        Serial.print("Ánh sáng: ");
        Serial.print(brightness);
        Serial.print("% - LED: ");
        Serial.println(ledState ? "BẬT" : "TẮT");
    }
    
    delay(100); // Giảm tải vòng lặp
}
