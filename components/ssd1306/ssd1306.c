/*
 * SSD1306 OLED display
 *
 * Allows control of an SSD1306 display using I2C protocol.
 *
 *  Copyright 2017 Sam Leitch
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "ssd1306.h"

struct ssd1306_s {
  i2c_port_t port;
};

struct ssd1306_s ssd1306[I2C_NUM_MAX];

ssd1306_t ssd1306_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl) {
  ssd1306_t ctx = ssd1306 + port;
  ctx->port = port;

  i2c_config_t i2c_conf;
  i2c_conf.mode = I2C_MODE_MASTER;
  i2c_conf.sda_io_num = sda;
  i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  i2c_conf.scl_io_num = scl;
  i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  i2c_conf.master.clk_speed = 400000;

  if(i2c_param_config(port, &i2c_conf)) return NULL;
  if(i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0)) return NULL;

  return ctx;
}

esp_err_t ssd1306_send_command(ssd1306_t ctx, uint8_t* command_bytes, size_t len, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, 0x78, true);
  i2c_master_write_byte(cmd, 0x00, true); // Single byte bit clear, Display RAM bit clear
  i2c_master_write(cmd, command_bytes, len, true);
  i2c_master_stop(cmd);
  esp_err_t result = i2c_master_cmd_begin(ctx->port, cmd, timeout);
  i2c_cmd_link_delete(cmd);
  return result;
}

esp_err_t ssd1306_send_graphic_data(ssd1306_t ctx, uint8_t* data_bytes, size_t len, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, 0x78, true);
  i2c_master_write_byte(cmd, 0x40, true); // Single byte bit clear, Display RAM bit set
  i2c_master_write(cmd, data_bytes, len, true);
  i2c_master_stop(cmd);
  esp_err_t result = i2c_master_cmd_begin(ctx->port, cmd, timeout);
  i2c_cmd_link_delete(cmd);
  return result;
}

esp_err_t ssd1306_send_page_data(ssd1306_t ctx, uint8_t page_start, uint8_t column_start, uint8_t* data_bytes, size_t len, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(page_start > 63) return ESP_ERR_INVALID_ARG;
  if(column_start > 127) return ESP_ERR_INVALID_ARG;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, 0x78, true);
  i2c_master_write_byte(cmd, 0x80, true); // Single byte bit set, Display RAM bit clear
  i2c_master_write_byte(cmd, 0x20, true); // Set memory address mode
  i2c_master_write_byte(cmd, 0x80, true); // Single byte bit set, Display RAM bit clear
  i2c_master_write_byte(cmd, address_mode_page, true); // memory address mode
  i2c_master_write_byte(cmd, 0x80, true); // Single byte bit set, Display RAM bit clear
  i2c_master_write_byte(cmd, 0x00 | (column_start & 0xf), true); // Column start low nibble
  i2c_master_write_byte(cmd, 0x80, true); // Single byte bit set, Display RAM bit clear
  i2c_master_write_byte(cmd, 0x00 | (column_start & 0xf), true); // Column start low nibble
  i2c_master_write_byte(cmd, 0x80, true); // Single byte bit set, Display RAM bit clear
  i2c_master_write_byte(cmd, 0x00 | (column_start >> 4), true); // Column start high nibble
  i2c_master_write_byte(cmd, 0x80, true); // Single byte bit set, Display RAM bit clear
  i2c_master_write_byte(cmd, 0xB0 | page_start, true); // page start
  i2c_master_write_byte(cmd, 0x40, true); // Single byte bit clear, Display RAM bit set
  i2c_master_write(cmd, data_bytes, len, true);
  i2c_master_stop(cmd);
  esp_err_t result = i2c_master_cmd_begin(ctx->port, cmd, timeout);
  i2c_cmd_link_delete(cmd);
  return result;
}

esp_err_t ssd1306_set_entire_display_on(ssd1306_t ctx, bool enabled, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { enabled ? 0xA5 : 0xA4 };
  return ssd1306_send_command(ctx, command_bytes, 1, timeout);
}

esp_err_t ssd1306_set_mux_ratio(ssd1306_t ctx, int mux_ratio, TickType_t timeout) {
  if(mux_ratio < 16) return ESP_ERR_INVALID_ARG;
  if(mux_ratio > 64) return ESP_ERR_INVALID_ARG;
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0xA8, (uint8_t)(mux_ratio - 1) };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_segment_remap(ssd1306_t ctx, bool enabled, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { enabled ? 0xA1 : 0xA0 };
  return ssd1306_send_command(ctx, command_bytes, 1, timeout);
}

esp_err_t ssd1306_set_display_start_line(ssd1306_t ctx, unsigned start_line, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(start_line > 63) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0x40 | start_line };
  return ssd1306_send_command(ctx, command_bytes, 1, timeout);
}

esp_err_t ssd1306_set_display_offset(ssd1306_t ctx, unsigned offset, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(offset > 63) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0xD3, (uint8_t)offset };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_reverse_scan_direction(ssd1306_t ctx, bool reverse, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { reverse ? 0xC8 : 0xC0 };
  return ssd1306_send_command(ctx, command_bytes, 1, timeout);
}

esp_err_t ssd1306_set_hardware_configuration(ssd1306_t ctx, bool alt_pin_assignement, bool left_right_remap, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t config_byte = 0x02;
  if(alt_pin_assignement) config_byte |= 0x10;
  if(left_right_remap) config_byte |= 0x20;
  uint8_t command_bytes[] = { 0xDA, config_byte };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_contract(ssd1306_t ctx, unsigned contrast, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(contrast > 255) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0x81, (uint8_t)contrast };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_display_inverse(ssd1306_t ctx, bool inverse, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { inverse ? 0xA7 : 0xA6 };
  return ssd1306_send_command(ctx, command_bytes, 1, timeout);
}

esp_err_t ssd1306_set_display_clock(ssd1306_t ctx, unsigned clk_divide, unsigned clk_freq, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(clk_divide == 0) return ESP_ERR_INVALID_ARG;
  if(clk_divide > 16) return ESP_ERR_INVALID_ARG;
  if(clk_freq > 15) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0xD5, (uint8_t)((clk_freq << 4) + clk_divide - 1) };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_memory_address_mode(ssd1306_t ctx, memory_address_mode_t mode, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0x20, mode };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_column_address(ssd1306_t ctx, unsigned start, unsigned end, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(start > 127) return ESP_ERR_INVALID_ARG;
  if(end > 127) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0x21, (uint8_t)start, (uint8_t)end };
  return ssd1306_send_command(ctx, command_bytes, 3, timeout);
}

esp_err_t ssd1306_set_page_address(ssd1306_t ctx, unsigned start, unsigned end, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(start > 7) return ESP_ERR_INVALID_ARG;
  if(end > 7) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0x22, (uint8_t)start, (uint8_t)end };
  return ssd1306_send_command(ctx, command_bytes, 3, timeout);
}

esp_err_t ssd1306_set_vcomh_deselect_level(ssd1306_t ctx, vcomh_deselect_t level, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0xDB, (uint8_t)(level << 4) };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_precharge_period(ssd1306_t ctx, unsigned phase1, unsigned phase2, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  if(phase1 == 0) return ESP_ERR_INVALID_ARG;
  if(phase1 > 15) return ESP_ERR_INVALID_ARG;
  if(phase2 == 0) return ESP_ERR_INVALID_ARG;
  if(phase2 > 15) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0xD9, (uint8_t)((phase2 << 4) + phase1) };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_charge_pump(ssd1306_t ctx, bool enabled, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { 0x8D, enabled ? 0x14 : 0x10 };
  return ssd1306_send_command(ctx, command_bytes, 2, timeout);
}

esp_err_t ssd1306_set_display_enabled(ssd1306_t ctx, bool enabled, TickType_t timeout) {
  if(ctx == NULL) return ESP_ERR_INVALID_ARG;
  uint8_t command_bytes[] = { enabled ? 0xAF : 0xAE };
  return ssd1306_send_command(ctx, command_bytes, 1, timeout);
}
