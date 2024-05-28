///////////////////////////////////////////////////////////////////////////////////////////
/*---------- Bagian Pengecekan Error ----------------------------------------------------*/
void function_checkError() {
  Serial.println("---- DEVICE STATUS ----\n");
  if (function_checkSdcard() == 0 || function_checkCamera() == 0) {
    Serial.print("\n\n in 10 seconds the program will be restarted \n");
    Serial.println("Countdown  : ");
    for (int x = 0; x < 10; x++) {
      digitalWrite(erFlashLed, 1);
      Serial.print(".");
      delay(1000);
    }
    ESP.restart();
  } else {
    Serial.print("\n\n program is run");
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
/*---------- check status sdcard --------------------------------------------------------*/
bool function_checkSdcard() {
  if (SD_MMC.begin() && cardType == CARD_NONE) {
    Serial.println("SD : Card Oke");
    return 1;
  } else {
    Serial.println("SD : Card Problem.....!");
    return 0;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
/*---------- check camera ---------------------------------------------------------------*/
bool function_checkCamera() {
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("esp : Camera Problem.....!");
    delay(1000);
    return 0;
  }
  Serial.println("esp : Camera Oke");
  esp_camera_fb_return(fb);
  return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////
bool function_checkNetwork() {
  /*kondisi saat dapat terkoneksi ---------------------------------------------------*/
  if (WiFi.status() == 3) {
    if (Client.connect("google.com", 80)) {
      Serial.print("WiFi : WIFI-ONLINE\n\n");
      Client.stop();
      return 1;
    }
    Serial.println("WiFi : WIFI-OFFLINE");
    return 0;
  }
  /*kondisi saat tidak dapat terkoneksi ----------------------------------------------*/
  else {
    Serial.println("WiFi : router connection lost");
    // Hubungkan ke wifi setiap timeout
    if ((millis() / 1000) - previous_time >= timeset) {
      previous_time = (millis() / 1000);
      Serial.println("WiFi : CONNECT");
      WiFi.begin(ssid, password);
      delay(5000);
    }
    return 0;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
camera_fb_t *function_takeImage() {
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  return fb;
}
void function_saveImagetoSdcard(camera_fb_t *inImage) {
  String path;  // untuk penamaan foto pada sdcard
  camera_fb_t *fb = NULL;
  fb = inImage;

  pictureNumber = EEPROM.read(0) + 1;
  path = "/eRboT/picture" + String(pictureNumber) + ".JPEG";
  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());
  File file = fs.open(path.c_str(), FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len);  // payload (image), payload length
    //    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }

  file.close();
  esp_camera_fb_return(fb);
  //rtc_gpio_hold_en(GPIO_NUM_4);
  support_listDir(SD_MMC, "/eRboT", 0);
}
///////////////////////////////////////////////////////////////////////////////////////////
/* void ini digunakan untuk melihat isi dari sdcard */
void support_listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        support_listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
String support_convert1toTrue(bool indata) {
  if (indata == 1) {
    return "enable\n";
  } else {
    return "disable\n";
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
/*fungsi akan melakukan pengecekan error setiap detik yang di tentukan guna memastikan
camera dalam kondisi oke */
void support_checkErrorCamara() {
  if ((millis() / 1000) - previous_time1 >= timeoutCheckESP) {
    previous_time1 = (millis() / 1000);
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      ESP.restart();
    }
  }
}