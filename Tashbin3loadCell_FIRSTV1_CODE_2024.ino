#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>
#include "HX711.h"

// พินสัญญาณสำหรับ Load Cell และ Relay
#define DOUT_1 D0
#define DOUT_2 D5
#define DOUT_3 D6
#define SCK D7

#define RELAY1 D1 
#define RELAY2 D2  
#define RELAY3 D3  
#define RELAY4 D4  

#define SSID        "zxcvbnm"
#define PASSWORD    "1234567890"
#define LINE_TOKEN  "xsHWA2MCWfpW7F30ujyY7lHjGC1dxU402oEGy1h5Vsa"

// สร้างวัตถุ HX711 สำหรับแต่ละถัง
HX711 scale1;
HX711 scale2;
HX711 scale3;

// ตัวแปรสถานะสำหรับแต่ละถัง
bool hasNotifiedCloseToFull1 = false, hasNotifiedFull1 = false, hasNotifiedEmpty1 = false;
bool hasNotifiedCloseToFull2 = false, hasNotifiedFull2 = false, hasNotifiedEmpty2 = false;
bool hasNotifiedCloseToFull3 = false, hasNotifiedFull3 = false, hasNotifiedEmpty3 = false;

void setup() {
  Serial.begin(9600);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(400); }

  LINE.setToken(LINE_TOKEN);

  // ตั้งค่า Load Cell
  scale1.begin(DOUT_1, SCK);
  scale2.begin(DOUT_2, SCK);
  scale3.begin(DOUT_3, SCK);
  
  scale1.set_offset(416795);
  scale1.set_scale(278.972717);
  scale1.tare();

  scale2.set_offset(450766);
  scale2.set_scale(140.335999);
  scale2.tare();

  scale3.set_offset(159783);
  scale3.set_scale(177.085968);
  scale3.tare();

  // ตั้งค่า Relay
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);

  LINE.notify("ระบบพร้อมใช้งาน");
}

void loop() {
  // อ่านน้ำหนักจาก Load Cell แต่ละตัว
  float weightKg1 = scale1.is_ready() ? scale1.get_units(15) / 1000.0 : 0.0;
  float weightKg2 = scale2.is_ready() ? scale2.get_units(15) / 1000.0 : 0.0;
  float weightKg3 = scale3.is_ready() ? scale3.get_units(15) / 1000.0 : 0.0;

  // ตรวจสอบถังขยะ
  handleTrashBin(1, weightKg1, hasNotifiedCloseToFull1, hasNotifiedFull1, hasNotifiedEmpty1, RELAY2);
  handleTrashBin(2, weightKg2, hasNotifiedCloseToFull2, hasNotifiedFull2, hasNotifiedEmpty2, RELAY3);
  handleTrashBin(3, weightKg3, hasNotifiedCloseToFull3, hasNotifiedFull3, hasNotifiedEmpty3, RELAY4);

  delay(1000);
}

// ฟังก์ชันจัดการสถานะและการสับ Relay
void handleTrashBin(int binId, float weightKg, bool &hasNotifiedCloseToFull, bool &hasNotifiedFull, bool &hasNotifiedEmpty, int relayX) {
  Serial.print("Trash Bin ");
  Serial.print(binId);
  Serial.print(": ");
  Serial.print(weightKg);
  Serial.println(" kg");

  // กรณีถังใกล้เต็ม
  if (weightKg >= 2.70 && weightKg < 4.00 && !hasNotifiedCloseToFull) {
    LINE.notify("ถังขยะ " + String(binId) + " ใกล้เต็มแล้ว");
    hasNotifiedCloseToFull = true;
    hasNotifiedFull = false;
    // ไม่สับ Relay
  }
  // กรณีถังเต็ม
  else if (weightKg >= 4.00 && !hasNotifiedFull) {
    LINE.notify("ถังขยะ " + String(binId) + " เต็มแล้ว กรุณานำไปเท");
    hasNotifiedFull = true;
    hasNotifiedEmpty = false;

    // เปิด Relay1 (ตัวกลาง) และ RelayX (ตามถัง)
    digitalWrite(RELAY1, HIGH);
    digitalWrite(relayX, HIGH);
  }
  // กรณีถังว่างหลังเท
  else if (weightKg < 1.00 && hasNotifiedFull && !hasNotifiedEmpty) {
    LINE.notify("ถังขยะ " + String(binId) + " ถูกนำไปเทแล้ว");
    hasNotifiedEmpty = true;
    hasNotifiedCloseToFull = false;
    hasNotifiedFull = false;

    // ปิด Relay1 (ตัวกลาง) และ RelayX (ตามถัง)
    digitalWrite(RELAY1, LOW);
    digitalWrite(relayX, LOW);
  }
}