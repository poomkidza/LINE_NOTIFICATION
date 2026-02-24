#include <WiFi.h>
#include <HTTPClient.h>

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

  // ส่งข้อความทักทายครั้งแรก
  //sendLineMessage("สวัสดีจาก ESP32 🎉\nระบบควบคุมไฟพร้อมแล้ว!");
}

bool lastOnState = HIGH;
bool lastOffState = HIGH;

void loop() {

  bool onState = digitalRead(BUTTON_ON_PIN);
  bool offState = digitalRead(BUTTON_OFF_PIN);

  if (lastOnState == HIGH && onState == LOW) {
    Serial.println("ON button pressed");

    digitalWrite(LED_PIN, HIGH);
    sendLineMessage("ส่งข้อความแล้ว");
  }

  if (lastOffState == HIGH && offState == LOW) {
    Serial.println("OFF button pressed");

    digitalWrite(LED_PIN, LOW);
    sendLineMessage("🔌 ปิดไฟแล้ว");
  }

  lastOnState = onState;
  lastOffState = offState;

  delay(20);
}


// === ฟังก์ชันส่งข้อความ ===
void sendLineMessage(String msg) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.line.me/v2/bot/message/push");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN);

    String payload = "{";
    payload += "\"to\":\"" + USER_ID + "\",";
    payload += "\"messages\":[{\"type\":\"text\",\"text\":\"" + msg + "\"}]";
    payload += "}";

    int code = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(code);

    if (code > 0) {
      String res = http.getString();
      Serial.println(res);
    } else {
      Serial.println("ส่งไม่สำเร็จ!");
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}
