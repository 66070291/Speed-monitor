#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

TinyGPSPlus gps;
SoftwareSerial gpsSerial(0, 1);  // RX, TX

#define BUZZER_PIN 7   // Buzzer
#define BUTTON 8  // ปุ่มสำหรับเริ่มวัดระยะทาง
#define LED_PIN 3

double lastlatitude = 0, lastlongtitude = 0;
double totaldistance = 0;

bool isMeasuring = false;  // ตรวจสอบสถานะการวัดระยะทาง

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  // จอ OLED เริ่มทำงาน
  if (gps.location.isUpdated()) {
  Serial.println("GPS location updated.");
  // ส่วนอื่น ๆ ตามปกติ
} else {
  Serial.println("Waiting for GPS signal...");
}

  if (!gps.location.isValid()) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Waiting for GPS...");
  display.display();
}

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Failed to initialize OLED"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(F("GPS Speed & Distance"));
  display.display();
  delay(2000);

  // กำหนด pin ของ buzzer และปุ่ม
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated()) {
      //for calcualtion
      double currentlatitude = gps.location.lat();
      double currentlongtitude = gps.location.lng();

      double speed = gps.speed.kmph();  // รับความเร็วปัจจุบัน
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.print("Speed: ");
      display.setCursor(0, 30);
      display.print(speed, 1);
      display.print(" km/h");
      Serial.println(speed);
      Serial.print(" km/h");
      //เช็คserial ละ ลอง
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
      display.display();

      if (speed > 30) {
        digitalWrite(BUZZER_PIN, LOW);  // เปิด buzzer
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(BUZZER_PIN, HIGH);   // ปิด buzzer
        digitalWrite(LED_PIN, LOW);
      }

      if(lastlatitude != 0 && lastlongtitude != 0 && isMeasuring){
        double distance = gps.distanceBetween(lastlatitude, lastlongtitude, currentlatitude, currentlongtitude)/10000; //calculate distance to km
        totaldistance += distance;
        Serial.print(" distance= "); 
        Serial.println(totaldistance);
      }

      //update lastposition
      lastlatitude = currentlatitude;
      lastlongtitude = currentlongtitude;

      // วัดระยะทางเมื่อกดปุ่ม
      int state = digitalRead(BUTTON);
      if (state == 0 && !isMeasuring) {
        // บันทึกพิกัดเริ่มต้น
        totaldistance = 0;
        isMeasuring = true;
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("Start measuring distance");
        Serial.println("Start measuring distance...");
        display.display();
        delay(3000);  // Debounce delay
      }else if (state == 0 && isMeasuring) {
        // บันทึกพิกัดปลายทาง
        isMeasuring = false;
        Serial.println("Stop measuring distance...");
      
        
        // แสดงระยะทางบนจอ
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("Result    Distance: ");
        display.setCursor(0, 45);
        display.print(totaldistance, 2);
        display.println(" km");
        display.display();
        
        Serial.print("Result Distance: ");
        Serial.print(totaldistance, 2);
        Serial.println(" km");
        delay(5000);  // Debounce delay
      }
    }
  }
}

