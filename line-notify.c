#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h"

// ===== WiFi =====
const char* ssid = "TangratPhone";
const char* password = "poomkidza";

// ===== LINE Messaging API =====
String CHANNEL_ACCESS_TOKEN = "0e3Rym2Nb9IkxT0mcaKhrUgW5FkbN6xhq2aUUbxCecPyXW3YKmx4eff0KYQTL6PkE3mAR8YHZYdnJLQcrMPgj5g8kW7ofJDyy/c3cm7TstaIstnUBZbYaK09NPZFtdXK++AHaeCI8norsOMRmWQGVgdB04t89/1O/w1cDnyilFU=";
String USER_ID = "U3e910e2f44a8b27943057e07d320c78e";

// ===== IFTTT Webhook =====
String iftttURL = "https://maker.ifttt.com/trigger/playmusic/with/key/K-v6ybbLvh2ohsbl3Yuct";

// ===== PIR =====
int pirPin = 2;

// ===== Time =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

unsigned long lastAlert = 0;
const unsigned long alertInterval = 15000; // 30 วินาที

// ---- escape JSON ----
String escapeJson(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\n", "\\n");
  return s;
}

void setup() {

  Serial.begin(115200);

  pinMode(pirPin, INPUT);

  Serial.println("Connecting WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting time...");
    delay(500);
  }

  Serial.println("Time synced");
}

void loop() {

  bool motion = digitalRead(pirPin);

  if (motion == HIGH && (millis() - lastAlert > alertInterval)) {

    lastAlert = millis();

    Serial.println("Motion Detected!");

    String msg =
      "แจ้งเตือน\nวันที่เวลา : " +
      getThaiTime() +
      "\nดูลิงก์ --> http://172.20.10.3";

    sendLineMessage(msg);
    triggerIFTTT();
  }

  delay(200);
}

// ===== ส่ง LINE =====
void sendLineMessage(String message) {

  message = escapeJson(message);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.line.me/v2/bot/message/push");

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN);

  String payload =
  "{\"to\":\"" + USER_ID + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";

  int httpCode = http.POST(payload);

  Serial.print("LINE Response: ");
  Serial.println(httpCode);

  Serial.println(http.getString());

  http.end();
}

// ===== เรียก IFTTT =====
void triggerIFTTT() {

  HTTPClient http;

  http.begin(iftttURL);

  int httpCode = http.GET();

  Serial.print("IFTTT Response: ");
  Serial.println(httpCode);

  http.end();
}

// ===== เวลาไทย =====
String getThaiTime() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) return "เวลาไม่ถูกต้อง";

  const char* wday[] = {
    "วันอาทิตย์","วันจันทร์","วันอังคาร",
    "วันพุธ","วันพฤหัสบดี","วันศุกร์","วันเสาร์"
  };

  const char* monthTH[] = {
    "มกราคม","กุมภาพันธ์","มีนาคม","เมษายน",
    "พฤษภาคม","มิถุนายน","กรกฎาคม","สิงหาคม",
    "กันยายน","ตุลาคม","พฤศจิกายน","ธันวาคม"
  };

  char timeBuff[20];
  strftime(timeBuff, sizeof(timeBuff), "%H:%M:%S", &timeinfo);

  int buddhistYear = timeinfo.tm_year + 1900 + 543;

  String t =
    String(wday[timeinfo.tm_wday]) + " ที่ " +
    String(timeinfo.tm_mday) + " " +
    String(monthTH[timeinfo.tm_mon]) + " " +
    String(buddhistYear) +
    "\nเวลา " + String(timeBuff) + " น.";

  return t;
}
