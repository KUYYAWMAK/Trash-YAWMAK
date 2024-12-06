#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>
#include "HX711.h"

// พินสัญญาณสำหรับ Load Cell แต่ละตัว
uint8_t dataPin1 = D7;
uint8_t clockPin1 = D6;
uint8_t dataPin2 = D5;
uint8_t clockPin2 = D2;
uint8_t dataPin3 = D0;
uint8_t clockPin3 = D1;

// พินสำหรับ Relay แต่ละตัว
#define RELAY2 D8
#define RELAY3 3

#define SSID        "แผ่เมตตา"
#define PASSWORD    "1234567890"
#define LINE_TOKEN  "xsHWA2MCWfpW7F30ujyY7lHjGC1dxU402oEGy1h5Vsa"

// สร้างวัตถุ HX711 สำหรับแต่ละถัง
HX711 scale1;
HX711 scale2;
HX711 scale3;

// ตัวแปรสถานะสำหรับแต่ละถัง
bool hasNotifiedLowFull1 = false, hasNotifiedMediumFull1 = false, hasNotifiedFull1 = false, hasNotifiedEmpty1 = false;
bool hasNotifiedLowFull2 = false, hasNotifiedMediumFull2 = false, hasNotifiedFull2 = false, hasNotifiedEmpty2 = false;
bool hasNotifiedLowFull3 = false, hasNotifiedMediumFull3 = false, hasNotifiedFull3 = false, hasNotifiedEmpty3 = false;

// กำหนดชื่อถังขยะ
const char *binNames[] = {"ถังขยะทั่วไป", "ถังขยะขวดพลาสติก", "ถังขยะขวดและกระป๋อง"};

void setup() {
  Serial.begin(9600);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
  }

  LINE.setToken(LINE_TOKEN);

  // เริ่มต้น Load Cell แต่ละตัว
  scale1.begin(dataPin1, clockPin1);
  scale2.begin(dataPin2, clockPin2);
  scale3.begin(dataPin3, clockPin3);

  // ตั้งค่า Load Cell แต่ละตัว
  scale1.set_offset(745098);
  scale1.set_scale(211.999008);
  scale1.tare(20);

  scale2.set_offset(719770);
  scale2.set_scale(223.998489);
  scale2.tare(20);

  scale3.set_offset(181523);
  scale3.set_scale(223.962357);
  scale3.tare(20);

  // ตั้งค่า Relay
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  // ปิด Relay ทั้งหมด
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);

  LINE.notify("ระบบพร้อมใช้งาน");
}

void loop() {
  // อ่านน้ำหนักจาก Load Cell แต่ละตัว
  float weightKg1 = scale1.is_ready() ? scale1.get_units(1) / 1000.0 : 0.0;
  float weightKg2 = scale2.is_ready() ? scale2.get_units(1) / 1000.0 : 0.0;
  float weightKg3 = scale3.is_ready() ? scale3.get_units(1) / 1000.0 : 0.0;

  // ตรวจสอบถัง
  handleTrashBin(0, weightKg1, hasNotifiedLowFull1, hasNotifiedMediumFull1, hasNotifiedFull1, hasNotifiedEmpty1, false);  // ถังขยะทั่วไป ไม่มีการเปิด Relay
  handleTrashBin(1, weightKg2, hasNotifiedLowFull2, hasNotifiedMediumFull2, hasNotifiedFull2, hasNotifiedEmpty2, RELAY2); // ถังขยะขวดพลาสติก
  handleTrashBin(2, weightKg3, hasNotifiedLowFull3, hasNotifiedMediumFull3, hasNotifiedFull3, hasNotifiedEmpty3, RELAY3); // ถังขยะขวดและกระป๋อง

  delay(1000);
}

// ฟังก์ชันจัดการสถานะและแจ้งเตือนของถังแต่ละถัง
void handleTrashBin(int binIndex, float weightKg, bool &hasNotifiedLowFull, bool &hasNotifiedMediumFull, bool &hasNotifiedFull, bool &hasNotifiedEmpty, int relayPin) {
  const char *binName = binNames[binIndex]; // ดึงชื่อถังขยะตาม Index

  Serial.print(binName);
  Serial.print(": ");
  Serial.print(weightKg);
  Serial.println(" kg");

  // เต็มเล็กน้อย
  if (weightKg >= 1.00 && weightKg < 1.50 && !hasNotifiedLowFull) {
    LINE.notify(String(binName) + " ขยะเต็มเล็กน้อย");
    hasNotifiedLowFull = true;
    hasNotifiedMediumFull = false;
    hasNotifiedFull = false;
    hasNotifiedEmpty = false;
  } 
  // เต็มปานกลาง
  else if (weightKg >= 1.50 && weightKg < 2.50 && !hasNotifiedMediumFull) {
    LINE.notify(String(binName) + " ขยะเต็มปานกลาง");
    hasNotifiedMediumFull = true;
    hasNotifiedFull = false;
    hasNotifiedLowFull = true; // ยังคงสถานะ LowFull
  } 
  // เต็มแล้ว
  else if (weightKg >= 2.50 && !hasNotifiedFull) {
    LINE.notify(String(binName) + " ขยะเต็มแล้ว กรุณานำไปเท");
    LINE.notify("https://www.google.com/maps/place/14.471730,100.112758");
    hasNotifiedFull = true;
    hasNotifiedEmpty = false;

    // เปิด Relay
    if (relayPin) {
      digitalWrite(relayPin, HIGH);
    }
  } 
  // ว่าง
  else if (weightKg < 1.00 && !hasNotifiedEmpty) {
    LINE.notify(String(binName) + " ถูกนำไปเทแล้ว");
    hasNotifiedEmpty = true;
    hasNotifiedLowFull = false;
    hasNotifiedMediumFull = false;
    hasNotifiedFull = false;

    // ปิด Relay
    if (relayPin) {
      digitalWrite(relayPin, LOW);
    }
  }
}