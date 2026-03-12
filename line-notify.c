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


///////////////////////////////////////////////////////////////
//Example ของ ESP32 Camera

#include "esp_camera.h"
#include <WiFi.h>

// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "TangratPhone";
const char *password = "poomkidza";

void startCameraServer();
void setupLedFlash();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash();
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}


