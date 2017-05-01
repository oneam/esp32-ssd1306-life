#include "cgol.h"
#include "driver/i2c.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "ssd1306.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void randomize(void* frame, size_t size) {
  uint32_t* frame32 = (uint32_t*)frame;
  int count32 = size / 4;
  for(int i = 0; i < count32; ++i) {
    frame32[i] = esp_random();
  }
}

void clear(void* frame, size_t size) {
  uint32_t* frame32 = (uint32_t*)frame;
  int count32 = size / 4;
  for(int i = 0; i < count32; ++i) {
    frame32[i] = 0;
  }
}

void app_main(void)
{
  nvs_flash_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  wifi_config_t sta_config = {
    .sta = {
      .ssid = "access_point_name",
      .password = "password",
      .bssid_set = false
    }
  };
  ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  ESP_ERROR_CHECK( esp_wifi_connect() );

  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_32, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  ESP_LOGI("main", "OLED display powered on");

  ssd1306_t ctx = ssd1306_init(I2C_NUM_0, GPIO_NUM_25, GPIO_NUM_26);
  ESP_LOGI("main", "OLED display I2C configured");

  TickType_t timeout = pdMS_TO_TICKS(1000);

  ESP_ERROR_CHECK( ssd1306_set_segment_remap(ctx, true, timeout) );
  ESP_ERROR_CHECK( ssd1306_set_reverse_scan_direction(ctx, true, timeout) );
  ESP_ERROR_CHECK( ssd1306_set_charge_pump(ctx, true, timeout) );
  ESP_ERROR_CHECK( ssd1306_set_display_enabled(ctx, true, timeout) );
  vTaskDelay(pdMS_TO_TICKS(100));
  ESP_LOGI("main", "OLED display initialized");

  cgol_t cgol = cgol_init(128, 64);

  {
    uint8_t* frame = cgol_get_state(cgol);
    randomize(frame, 1024);

    ESP_ERROR_CHECK( ssd1306_set_memory_address_mode(ctx, address_mode_horizontal, timeout) );
    ESP_ERROR_CHECK( ssd1306_set_page_address(ctx, 0, 7, timeout) );
    ESP_ERROR_CHECK( ssd1306_set_column_address(ctx, 0, 127, timeout) );
    ESP_ERROR_CHECK( ssd1306_send_graphic_data(ctx, frame, 1024, timeout) );
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  while(true) {
    uint8_t* frame = cgol_get_state(cgol);

    ESP_ERROR_CHECK( ssd1306_set_memory_address_mode(ctx, address_mode_horizontal, timeout) );
    ESP_ERROR_CHECK( ssd1306_set_page_address(ctx, 0, 7, timeout) );
    ESP_ERROR_CHECK( ssd1306_set_column_address(ctx, 0, 127, timeout) );
    ESP_ERROR_CHECK( ssd1306_send_graphic_data(ctx, frame, 1024, timeout) );

    cgol_take_turn(cgol);
  }

  ESP_LOGI("main", "Loop ended");
}

