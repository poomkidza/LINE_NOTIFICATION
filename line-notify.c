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

// ===== PIR =====
int pirPin = 2;

// ===== Time (NTP) =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

// ---- escape JSON (แก้ปัญหา 400) ----
String escapeJson(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\n", "\\n");
  return s;
}

void setup() {
  Serial.begin(115200);

  pinMode(pirPin, INPUT);

  Serial.println("Connecting WiFi.......");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // sync time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time syncing...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting time...");
    delay(500);
  }
  Serial.println("Time synced!");
}

void loop() {
  if (digitalRead(pirPin) == HIGH) {
    Serial.println("Motion Detected!");

    String msg = "แจ้งเตือน\nวันที่เวลา : " + getThaiTime() + "\nคลิ๊กที่นี่เพื่อดูลิ้งค์ --> http://172.20.10.3";
    sendLineMessage(msg);

    delay(3000);
  }
}

// ===== ส่งข้อความเข้า LINE Messaging API =====
void sendLineMessage(String message) {

  message = escapeJson(message);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.line.me/v2/bot/message/push");

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN);

  // สร้าง Payload
  String payload =
    "{\"to\":\"" + USER_ID + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";

  int httpCode = http.POST(payload);

  Serial.print("LINE Response: ");
  Serial.println(httpCode);
  Serial.println(http.getString());

  http.end();
}

// ===== เวลาแบบไทย =====
String getThaiTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "เวลาไม่ถูกต้อง";

  const char* wday[] = {
    "วันอาทิตย์", "วันจันทร์", "วันอังคาร",
    "วันพุธ", "วันพฤหัสบดี", "วันศุกร์", "วันเสาร์"
  };
  const char* monthTH[] = {
    "มกราคม", "กุมภาพันธ์", "มีนาคม", "เมษายน", "พฤษภาคม", "มิถุนายน",
    "กรกฎาคม", "สิงหาคม", "กันยายน", "ตุลาคม", "พฤศจิกายน", "ธันวาคม"
  };

  char timeBuff[20];
  strftime(timeBuff, sizeof(timeBuff), "%H:%M:%S", &timeinfo);

  int buddhistYear = timeinfo.tm_year + 1900 + 543;

  String t =
    String(wday[timeinfo.tm_wday]) + " ที่ " + String(timeinfo.tm_mday) + " " + String(monthTH[timeinfo.tm_mon]) + " " + String(buddhistYear) + "\nเวลา " + String(timeBuff) + " น.";

  return t;
}
