/*
    add    : Rahmat Fanshuri
    Versi  : 1.0
    Update : 12/12/2022
    code   : send picture telegram
    ket    :
    - Alamat EEPROM (0) di gunakan untuk penamaan gambar
*/
///////////////////////////////////////////////////////////////////////////////////////////
/* ---------- HEADER GLOGAL ----------------------------------------------------------*/
#include "Arduino.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <EEPROM.h>  // read and write from flash memory
#include <WiFiClientSecure.h>
#define EEPROM_SIZE 1
#define erFlashLed 4
/* ---------- HEADER ESP32-CAM -------------------------------------------------------*/
#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#define CAMERA_MODEL_AI_THINKER
uint8_t cardType = SD_MMC.cardType();
/* ---------- HEADER TELEGRAM --------------------------------------------------------*/
#include <UniversalTelegramBot.h>
/* ---------- PARAMETER WiFi ---------------------------------------------------------*/
WiFiClient Client;
WiFiClientSecure clientTCP;
const char* ssid = "";
const char* password = "";
const uint16_t port = 443;
const char* host = "8.8.4.4";  // ip or dns
/* ---------- PARAMETER TELEGRAM -----------------------------------------------------*/
String BOTtoken = "";
String CHAT_ID = "";
UniversalTelegramBot bot(BOTtoken, clientTCP);
/* ---------- PARAMETER GLOBAL -------------------------------------------------------*/
bool takeStatus, takeInfo, takeImage, takeFlash, takeMsg, statusFlash;
unsigned long previous_time, previous_time1;
unsigned long timeset = 10;  // set time in seconds
unsigned long timeoutCheckESP = 20;
unsigned long lastTimeBotRan;
int botRequestDelay = 1000;
String path;  // untuk penamaan foto pada sdcard
byte pictureNumber = 0;
bool sendPhoto = false;
///////////////////////////////////////////////////////////////////////////////////////////
#include "headerTelegram.h"
void setup() {
  /* ---------- begin ------------------------------------ */
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("\n\n\n\n  INI ESP32-CAM");
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  /* ---------- setting I/O ------------------------------- */
  pinMode(erFlashLed, OUTPUT);
  /* ---------- setting WiFi ------------------------------ */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(10000);
  /* ---------- setting esp32-cam ------------------------ */
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1);

  /*------------------------------------------------------------------*/
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  function_checkError();
  digitalWrite(erFlashLed, 1);
  delay(1000);
  digitalWrite(erFlashLed, 0);
  /*------------------------------------------------------------------*/
  bot.sendMessage(CHAT_ID, "Device On", "");
}

///////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  /* ---------- check camera ------------------------------------------------------*/
  support_checkErrorCamara();
  if (function_checkNetwork() == 1) {
    telegram_readMessage();
    proses_takeData();
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
void proses_takeData() {
  if (takeFlash == true) {
    statusFlash = !statusFlash;
    digitalWrite(erFlashLed, statusFlash);
    takeFlash = false;
  }
  if (takeInfo == true) {
    telegram_messageInfo();
    takeInfo = false;
  }
  if (takeStatus == true) {
    telegram_messageInfo();
    takeStatus = false;
  }
  if (takeMsg == true) {
    telegram_messageInfo();
    takeMsg = false;
  }
  if (takeImage == true) {
    camera_fb_t* result = NULL;
    bool resultCek = function_checkCamera();

    if (resultCek == 1) {
      result = function_takeImage();
      function_saveImagetoSdcard(result);
      telegram_sendImage(result);
    } else {
      bot.sendMessage(CHAT_ID, "camera bad", "");
    }
    takeImage = false;
  }
}
