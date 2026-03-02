#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

// === WiFi Config ===
#define ssid  "TangratPhone"
#define password "poomkidza"

// === LINE Messaging API Token ===
String CHANNEL_ACCESS_TOKEN = "0e3Rym2Nb9IkxT0mcaKhrUgW5FkbN6xhq2aUUbxCecPyXW3YKmx4eff0KYQTL6PkE3mAR8YHZYdnJLQcrMPgj5g8kW7ofJDyy/c3cm7TstaIstnUBZbYaK09NPZFtdXK++AHaeCI8norsOMRmWQGVgdB04t89/1O/w1cDnyilFU=";
String USER_ID = "U3e910e2f44a8b27943057e07d320c78e"; // คนหรือกลุ่มปลายทาง

// === กำหนดขา ปุ่ม และ ไฟ ===
const int BUTTON_ON_PIN  = 4;   // ปุ่มเปิดไฟ
const int BUTTON_OFF_PIN = 5;   // ปุ่มปิดไฟ
const int LED_PIN        = 2;   // ไฟที่ควบคุม
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);

  // ตั้งค่า I/O
  pinMode(BUTTON_ON_PIN, INPUT_PULLUP);   // ปุ่มต่อไป GND
  pinMode(BUTTON_OFF_PIN, INPUT_PULLUP);  // ปุ่มต่อไป GND
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);             // เริ่มต้นปิดไฟ

  // ต่อ WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  printf("WiFi MAC address is %s\n", WiFi.macAddress().c_str());
  printf("Connecting to WiFi %s\n", ssid);

  Serial.println("Connecting WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for time to sync....");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(1000);
  }
    Serial.println("Time synced!");


  // ส่งข้อความทักทายครั้งแรก
  delay(2000);
  sendLineMessage(
    "แจ้งเตือน\nวันที่เวลา : " + getThaiDateTime() +
    "\nผู้ป่วยแสดงอาการ คลิกเพื่อดูกล้อง -->"
  );
}

void loop(){
  
}


// === ฟังก์ชันส่งข้อความ ===
#include <WiFiClientSecure.h>

void sendLineMessage(String msg) {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();   // ⭐ สำคัญที่สุด

  HTTPClient http;

  http.begin(client,
    "https://api.line.me/v2/bot/message/push");

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization",
                 "Bearer " + CHANNEL_ACCESS_TOKEN);

  msg.replace("\"", "'");
  msg.replace("\n", "\\n");
  msg.replace("\r", "");

  String payload =
    "{\"to\":\"" + USER_ID +
    "\",\"messages\":[{\"type\":\"text\",\"text\":\"" +
    msg + "\"}]}";

  Serial.println("Sending...");
  Serial.println(payload);

  int httpCode = http.POST(payload);

  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  String response = http.getString();
  Serial.println(response);

  http.end();
}

String getDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "ไม่พบเวลา";
  }

  char buffer[50];
  strftime(buffer, sizeof(buffer),
          "%d/%m/%Y %H:%M:%S", &timeinfo);

  return String(buffer);
}

String getThaiDateTime() {

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "ไม่พบเวลา";
  }

  String dayTH[] = {
    "วันอาทิตย์","วันจันทร์","วันอังคาร",
    "วันพุธ","วันพฤหัสบดี","วันศุกร์","วันเสาร์"
  };

  String monthTH[] = {
    "มกราคม","กุมภาพันธ์","มีนาคม","เมษายน",
    "พฤษภาคม","มิถุนายน","กรกฎาคม","สิงหาคม",
    "กันยายน","ตุลาคม","พฤศจิกายน","ธันวาคม"
  };

  int day = timeinfo.tm_mday;
  int month = timeinfo.tm_mon;
  int year = timeinfo.tm_year + 1900 + 543; // พ.ศ.

  char timeBuffer[10];
  strftime(timeBuffer, sizeof(timeBuffer),
           "%H:%M:%S", &timeinfo);

  String result =
      dayTH[timeinfo.tm_wday] + " ที่ " +
      String(day) + " " +
      monthTH[month] + " " +
      String(year) +
      "\nเวลา " + String(timeBuffer) + " น.";

  return result;
}
