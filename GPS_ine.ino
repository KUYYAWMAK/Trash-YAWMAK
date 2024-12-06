#include <TridentTD_LineNotify.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define SSID        "แผ่เมตตา"                                     // ชื่อ WiFi
#define PASSWORD    "1234567890"                                   // รหัสผ่าน WiFi
#define LINE_TOKEN  "vIin177nkqBlLhIcqUMHOMoWuZId0LKqCgFlfdfO1yn" // Line Notify Token

#define RXPin 4  // GPS RX pin (to GPS TX)
#define TXPin 5  // GPS TX pin (to GPS RX)

// Create SoftwareSerial object
SoftwareSerial gpsSerial(RXPin, TXPin);

// Create TinyGPS++ object
TinyGPSPlus gps;
                     
void setup() {
  Serial.begin(9600); 
  gpsSerial.begin(9600); // Start GPS serial communication
  
  Serial.println("Starting...");

  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n", SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.printf("\nWiFi connected\nIP: ");
  Serial.println(WiFi.localIP());

  // Set Line Notify Token
  LINE.setToken(LINE_TOKEN);
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) { // Parse GPS data
      displayGPSData();
    }
  }
  delay(100);
}

void displayGPSData() {
  // Check if location data is valid
  if (gps.location.isValid()) {
    float la = gps.location.lat();  // Get latitude
    float lon = gps.location.lng(); // Get longitude
    
    // Print GPS data to Serial Monitor0
    Serial.print("Latitude: ");
    Serial.print(la, 6);
    Serial.print(" | Longitude: ");
    Serial.println(lon, 6);

    
    String message = "https://www.google.com/maps/place/";
    message += String(la, 6) + "," + String(lon, 6);

    
    LINE.notify(message);
  } else {
    Serial.println("Waiting for valid GPS data...");
  }
}