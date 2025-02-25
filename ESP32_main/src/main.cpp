#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <ESP32Servo.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "FFF";
const char* password = "00000000";

// LINE Notify credentials
String tokens[3] = {
  "bdzhRhvjviI8qUbrQMj8K6wVLHmBajfhdhz11wPyFLU",
  "pcMpeacnIigeS6HdOWQmuBkQf2Gbtl1VE9wiM9MPfJj",
  "wRBfrgQ86KKbPI2wNpcbjg5zWe1x6DDuYOEoAwiylWv"
};
String url = "https://notify-api.line.me/api/notify";

// ตั้งค่าพอร์ตสำหรับลายนิ้วมือ
HardwareSerial mySerial(1); // Serial1 ใช้ TX/RX ที่กำหนดเอง
Adafruit_Fingerprint finger(&mySerial);
HardwareSerial espCamSerial(2); // ใช้ Serial1 สำหรับเชื่อมกับ ESP32-CAM
#define C_RX_PIN 25                // Pin RX ของ ESP32 เชื่อมกับ TX ของ ESP32-CAM
#define C_TX_PIN 26                // Pin TX ของ ESP32 เชื่อมกับ RX ของ ESP32-CAM


// ตั้งค่าพอร์ตสำหรับ Servo
Servo myServo; // สร้างออบเจ็กต์ Servo
#define SERVO_PIN 12 // พอร์ตที่เชื่อมกับ Servo

#define TX_PIN 22
#define RX_PIN 23

// ตั้งค่าพอร์ตสำหรับ LED
#define RED_LED_PIN 27    // ไฟแดง
#define YELLOW_LED_PIN 33 // ไฟเหลือง
#define GREEN_LED_PIN 32  // ไฟเขียว
#define ULTRA_LED_PIN 2   // ไฟ LED สำหรับแสง

// ตั้งค่าพอร์ตสำหรับ Ultrasonic Sensor
#define TRIG_PIN 18
#define ECHO_PIN 19

// ตั้งค่าพอร์ตสำหรับ PIR Motion Sensor
#define PIR_PIN 5

// ตั้งค่าพอร์ตสำหรับ LDR
#define LDR_PIN 35 // GPIO15 สำหรับอ่านค่า LDR
#define LDR_THRESHOLD 4095 // ค่าความสว่างที่ใช้เป็นเกณฑ์



// ตัวแปรสถานะ
int status = 0; // เริ่มต้นสถานะเป็น "ไม่พร้อม"

// List ของ ID และ Name
struct User {
  int id;
  const char* name;
};
User users[] = {
  {1, "Chetsada Kongsipan"},
  {2, "Huracan"},
  {3, "Chayanin"},
  {4, "Sirawich"}
};
const int numUsers = sizeof(users) / sizeof(users[0]); // จำนวนผู้ใช้ใน List

void updateLED(int status);
long getUltrasonicDistance();
bool isMotionDetected();
int getFingerprintID();
const char* getUserName(int id);
void updateUltraLED(); // ฟังก์ชันอัปเดต LED จาก LDR

void setup() {
  Serial.begin(9600); // Serial Monitor
  mySerial.begin(57600, SERIAL_8N1, TX_PIN, RX_PIN); // TX=22, RX=23
  espCamSerial.begin(9600, SERIAL_8N1, C_RX_PIN, C_TX_PIN); // ตั้งค่า Serial1
  if (espCamSerial.available()) {
        String response = espCamSerial.readStringUntil('\n');
        response.trim();
        Serial.println(response);
  }
  status = 0; // พร้อม
  updateLED(status);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  delay(5000);
  espCamSerial.println("START");
  delay(500);
  
  // เริ่มต้นเซ็นเซอร์ลายนิ้วมือ
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
  } else {
    Serial.println("Fingerprint sensor not found!");
    while (1) { delay(1); }
  }

  // ตั้งค่า Servo
  myServo.attach(SERVO_PIN);
  myServo.write(0); // เริ่มที่ตำแหน่ง Lock (0°)

  // ตั้งค่า LED
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(ULTRA_LED_PIN, OUTPUT);
  updateLED(status); // ตั้งค่าสถานะเริ่มต้น (ไม่พร้อม)

  // ตั้งค่า Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // ตั้งค่า PIR Motion Sensor
  pinMode(PIR_PIN, INPUT);

  // ตั้งค่า LDR
  pinMode(LDR_PIN, INPUT);


}

void sendLineMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    for (int i = 0; i < 3; i++) { // Loop through each token
      http.begin("https://notify-api.line.me/api/notify");

      // Set headers
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      http.addHeader("Authorization", "Bearer " + tokens[i]);

      // Send POST request
      int httpResponseCode = http.POST("message=" + message);

      // Print response
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code for token ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error on sending POST for token ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
      }
      http.end(); // End the connection
      delay(500); // Short delay to prevent server overload
    }
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  if (espCamSerial.available()) {
        String response = espCamSerial.readStringUntil('\n');
        response.trim();
        Serial.println(response);
        if (response.indexOf("Camera Ready! Use") != -1) {
          sendLineMessage(response);
        }
  }
  if (status == 0) { // ไม่พร้อม (ไฟแดง)
    digitalWrite(ULTRA_LED_PIN, LOW);
    if (isMotionDetected()) {
      unsigned long startTime = millis();
      bool isReady = false;

      while (millis() - startTime < 60000) { // ตรวจจับ 1 นาที
        long distance = getUltrasonicDistance();
        if (distance < 50) {
          isReady = true;
          break;
        }
        delay(500); // รอ 0.5 วินาทีเพื่อลดโหลด
      }

      if (isReady) {
        status = 1; // พร้อม
        updateLED(status);
      }
    }
  } else if (status == 1) { // พร้อม (ไฟเหลือง)
    // อัปเดต LED ของ ULTRA_LED จาก LDR
    updateUltraLED();
    delay(500);
    unsigned long lastMotionTime = millis();
    while (true) { // เช็คเงื่อนไขสถานะพร้อมต่อเนื่อง
      updateUltraLED(); // อัปเดต ULTRA_LED
      long distance = getUltrasonicDistance();
      if (isMotionDetected() && distance < 50) { // มีการเคลื่อนไหว รีเซ็ตเวลา
        lastMotionTime = millis();
      }

      if (millis() - lastMotionTime >= 60000) {
        // ไม่มีการเคลื่อนไหว หรือ Ultrasonic อยู่ห่างเกิน 50 ซม. 1 นาที
        status = 0; // เปลี่ยนเป็นสถานะไม่พร้อม
        updateLED(status);
        break; // ออกจากสถานะพร้อม
      }

      // ตรวจสอบคำตอบจาก ESP32-CAM
      if (espCamSerial.available()) {
        String response = espCamSerial.readStringUntil('\n');
        response.trim();
        Serial.println(response);
        if (response.indexOf("Camera Ready! Use") != -1) {
          sendLineMessage(response);
        }
        if (response.indexOf("Match Face ID:") != -1) {
          Serial.println("Unlock With FaceID");
          sendLineMessage("Unlocks with FaceID");
          myServo.write(90); // ปลดล็อกประตู
          // เปลี่ยนสถานะเป็น "สแกนผ่าน"
          status = 2;
          updateLED(status); // อัปเดตไฟ LED
          delay(10000); // รอ 10 วินาที

          myServo.write(0); // ล็อกประตู
          Serial.println("Door locked.");

          // กลับสู่สถานะพร้อม
          status = 1;
          updateLED(status);
          delay(500);
        }
      }
      // ตรวจสอบลายนิ้วมือในสถานะพร้อม
      int id = getFingerprintID();
      if (id != -1) {
        digitalWrite(ULTRA_LED_PIN, LOW);
        const char* name = getUserName(id); // ดึงชื่อจาก ID
        Serial.print("Fingerprint matched! ID: ");
        Serial.print(id);
        Serial.print(", Name: ");
        Serial.println(name ? name : "Unknown");
        sendLineMessage(String(name) + " unlocks with fingerprint.");
        // เปลี่ยนสถานะเป็น "สแกนผ่าน"
        status = 2;
        updateLED(status); // อัปเดตไฟ LED

        // ปลดล็อกประตู
        myServo.write(90); // หมุน Servo ไปที่ตำแหน่ง Unlock (90°)
        Serial.println("Door unlocked.");
        delay(10000); // รอ 10 วินาที

        // ล็อกกลับ
        myServo.write(0);  // หมุน Servo กลับไปที่ตำแหน่ง Lock (0°)
        Serial.println("Door locked.");

        // กลับสู่สถานะพร้อม
        status = 1;
        updateLED(status);
      }

      delay(500); // รอ 0.5 วินาทีเพื่อลดโหลด
    }
  }

}

// ฟังก์ชันอัปเดต LED
void updateLED(int status) {
  switch (status) {
    case 0: // ไม่พร้อม (ไฟแดง)
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      break;
    case 1: // พร้อม (ไฟเหลือง)
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      break;
    case 2: // สแกนผ่าน (ไฟเขียว)
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      break;
  }
}

// ฟังก์ชันอัปเดต ULTRA_LED จาก LDR
void updateUltraLED() {
  int ldrValue = analogRead(LDR_PIN);
  if (ldrValue == LDR_THRESHOLD) {
    digitalWrite(ULTRA_LED_PIN, HIGH);
    
  } else {
    digitalWrite(ULTRA_LED_PIN, LOW);
  }
}

// ฟังก์ชันคำนวณระยะจาก Ultrasonic Sensor
long getUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2; // คำนวณระยะเป็นเซนติเมตร
  return distance;
}

// ฟังก์ชันตรวจจับการเคลื่อนไหวจาก PIR
bool isMotionDetected() {
  Serial.print(digitalRead(PIR_PIN));
  return digitalRead(PIR_PIN) == HIGH;
}

// ฟังก์ชันตรวจสอบลายนิ้วมือ
int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    return -1; // ไม่มีลายนิ้วมือ
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    return -1; // แปลงภาพลายนิ้วมือล้มเหลว
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    return -1; // ไม่พบลายนิ้วมือในฐานข้อมูล
  }

  return finger.fingerID; // คืนค่า ID ของลายนิ้วมือ
}

// ฟังก์ชันค้นหาชื่อจาก ID
const char* getUserName(int id) {
  for (int i = 0; i < numUsers; i++) {
    if (users[i].id == id) {
      return users[i].name;
    }
  }
  return nullptr; // หากไม่พบ ID ใน List
}