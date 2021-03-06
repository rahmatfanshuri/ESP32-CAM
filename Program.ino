#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#define led 4
#define led_green 14
#define led_red 15


unsigned long currentMillis = 0;
unsigned long openedMillis = 0;
long interval = 5000;
unsigned int Status;

static inline mtmn_config_t app_mtmn_config()
{
  mtmn_config_t mtmn_config = {0};
  mtmn_config.type = FAST;
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;
  return mtmn_config;
}

mtmn_config_t mtmn_config = app_mtmn_config();
static face_id_name_list st_face_list;
dl_matrix3du_t *image_matrix =  NULL;
camera_fb_t * fb = NULL;
dl_matrix3du_t *aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);

void setup() {
  Serial.begin(115200);
  Serial.println(" This ESP32-CAM");
  pinMode(led, OUTPUT); digitalWrite(led, 0);
  pinMode(led_green, OUTPUT); digitalWrite(led_green, 0);
  pinMode(led_red, OUTPUT); digitalWrite(led_red, 1);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5; config.pin_d1 = 18;
  config.pin_d2 = 19; config.pin_d3 = 21;
  config.pin_d4 = 36; config.pin_d5 = 39;
  config.pin_d6 = 34; config.pin_d7 = 35;
  config.pin_xclk = 0; config.pin_pclk = 22;
  config.pin_vsync = 25; config.pin_href = 23;
  config.pin_sscb_sda = 26; config.pin_sscb_scl = 27;
  config.pin_pwdn = 32; config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
}


void loop() {
  run_face_detect();
  if (Status == 1) {
    digitalWrite(led_green, 1);
    digitalWrite(led_red, 0);
    delay(1000);
    Status = 0;
  }
  else {
    digitalWrite(led_green, 0);
    digitalWrite(led_red, 1);
  }

}

bool run_face_detect() {
  bool faceRecognised = false;
  int64_t start_time = esp_timer_get_time();
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }

  int64_t fb_get_time = esp_timer_get_time();
  image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  uint32_t res = fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);

  if (!res)
  {
    Serial.println("to rgb888 failed");
    dl_matrix3du_free(image_matrix);
  }

  esp_camera_fb_return(fb);
  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

  if (net_boxes) {

    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK)
    {
      Serial.println("Face Detected");
      Status = 1;
    }
    else
    {
      Serial.println("Face Not Aligned");
    }
    free(net_boxes->box); free(net_boxes->landmark); free(net_boxes);
  }

  else
  {
    Serial.println("No Face Detected");
  }

  dl_matrix3du_free(image_matrix);
  return faceRecognised;
}
