#include "driver/i2s.h"
#define I2S_WS_MIC 15
#define I2S_SCK_MIC 14
#define I2S_SD_MIC 32
#define I2S_WS_SPK 27
#define I2S_BCK_SPK 26
#define I2S_DATA_SPK 25
#define BTN_RECORD 0
#define LED 2
#define SAMPLE_RATE 16000
#define RECORD_TIME 3
#define I2S_PORT_MIC I2S_NUM_0
#define I2S_PORT_SPK I2S_NUM_1
#define SAMPLES_COUNT (SAMPLE_RATE * RECORD_TIME)
int16_t samples[SAMPLES_COUNT];
int sampleCount = 0;
void micSetup() {
  i2s_config_t cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  cfg.sample_rate = SAMPLE_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  cfg.use_apll = false;
  i2s_pin_config_t pins;
  memset(&pins, 0, sizeof(pins));
  pins.bck_io_num = I2S_SCK_MIC;
  pins.ws_io_num = I2S_WS_MIC;
  pins.data_out_num = -1;
  pins.data_in_num = I2S_SD_MIC;
  i2s_driver_install(I2S_PORT_MIC, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT_MIC, &pins);
}
void spkSetup() {
  i2s_config_t cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  cfg.sample_rate = SAMPLE_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  cfg.use_apll = false;
  cfg.tx_desc_auto_clear = true;
  i2s_pin_config_t pins;
  memset(&pins, 0, sizeof(pins));
  pins.bck_io_num = I2S_BCK_SPK;
  pins.ws_io_num = I2S_WS_SPK;
  pins.data_out_num = I2S_DATA_SPK;
  pins.data_in_num = -1;
  i2s_driver_install(I2S_PORT_SPK, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT_SPK, &pins);
}
void setup() {
  Serial.begin(115200);
  pinMode(BTN_RECORD, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  micSetup();
  spkSetup();
  Serial.println("Ready! Press BOOT");
}
void loop() {
  if (digitalRead(BTN_RECORD) == LOW) {
    delay(50);
    if (digitalRead(BTN_RECORD) == LOW) {
      digitalWrite(LED, LOW);
      Serial.println("Recording...");
      sampleCount = 0;
      size_t bytes;
      int16_t buf[256];
      uint32_t end = millis() + RECORD_TIME * 1000;
      while (millis() < end) {
        i2s_read(I2S_PORT_MIC, buf, sizeof(buf), &bytes, 100);
        int n = bytes / 2;
        for (int i = 0; i < n && sampleCount < SAMPLES_COUNT; i++)
          samples[sampleCount++] = buf[i];
      }
      Serial.println("Playing...");
      size_t bw;
      int pos = 0;
      while (pos < sampleCount) {
        int n = min(256, sampleCount - pos);
        i2s_write(I2S_PORT_SPK, &samples[pos], n*2, &bw, 100);
        pos += n;
      }
      Serial.println("Done!");
      digitalWrite(LED, HIGH);
      delay(500);
    }
  }
}
