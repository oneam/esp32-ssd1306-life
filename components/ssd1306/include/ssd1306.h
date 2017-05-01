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

#ifndef COMPONENTS_SSD1306_H_
#define COMPONENTS_SSD1306_H_

#include <stddef.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"

/* SSD1306 address mode */
typedef enum memory_address_mode_e {
  address_mode_horizontal,
  address_mode_vertical,
  address_mode_page
} memory_address_mode_t;

/* Values for Vcomh */
typedef enum vcomh_deselect_e {
  vcomh_deselect_0_65Vcc = 0,
  vcomh_deselect_0_77Vcc = 2,
  vcomh_deselect_0_83Vcc = 3,
} vcomh_deselect_t;

/* Opaque implementation pointer */
typedef struct ssd1306_s* ssd1306_t;

/* Initialize the I2C port and driver */
ssd1306_t ssd1306_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

/* Send graphic data to the device */
esp_err_t ssd1306_send_graphic_data(ssd1306_t ctx, uint8_t* data_bytes, size_t len, TickType_t timeout);

/* Send a single command or string of commands to the device. */
esp_err_t ssd1306_set_command(ssd1306_t ctx, uint8_t* command_bytes, size_t len, TickType_t timeout);

/* Sends data to a single page starting at a given column offset. (page_start must be 0-7, column_start 0-127) */
esp_err_t ssd1306_send_page_data(ssd1306_t ctx, uint8_t page_start, uint8_t column_start, uint8_t* data_bytes, size_t len, TickType_t timeout);

/* Display charge pump used for input voltage less than 7.5V */
esp_err_t ssd1306_set_charge_pump(ssd1306_t ctx, bool enabled, TickType_t timeout);

/* Display enabled or put in low-power sleep mode */
esp_err_t ssd1306_set_display_enabled(ssd1306_t ctx, bool enabled, TickType_t timeout);

/* Turn on all pixels of the display */
esp_err_t ssd1306_set_entire_display_on(ssd1306_t ctx, bool enabled, TickType_t timeout);

/* Display multiplex ratio (mux_ratio must be 1-64) */
esp_err_t ssd1306_set_mux_ratio(ssd1306_t ctx, int mux_ratio, TickType_t timeout);

/* Flips the display horizontally */
esp_err_t ssd1306_set_segment_remap(ssd1306_t ctx, bool enabled, TickType_t timeout);

/* First line of graphics data that will appear on the display (start_line must be 0-63) */
esp_err_t ssd1306_set_display_start_line(ssd1306_t ctx, unsigned start_line, TickType_t timeout);

/* Shift the display a number of lines (offset must be 0-63) */
esp_err_t ssd1306_set_display_offset(ssd1306_t ctx, unsigned offset, TickType_t timeout);

/* Flips the screen vertically */
esp_err_t ssd1306_set_reverse_scan_direction(ssd1306_t ctx, bool reverse, TickType_t timeout);

/* Alters the mapping of graphics data to display */
esp_err_t ssd1306_set_hardware_configuration(ssd1306_t ctx, bool alt_pin_assignment, bool left_right_remap, TickType_t timeout);

/* Sets display contrast (contrast must be 0-255) */
esp_err_t ssd1306_set_contract(ssd1306_t ctx, unsigned contrast, TickType_t timeout);

/* When inverse, 0 bits display as on */
esp_err_t ssd1306_set_display_inverse(ssd1306_t ctx, bool inverse, TickType_t timeout);

/* Used to change the refresh rate of the display */
esp_err_t ssd1306_set_display_clock(ssd1306_t ctx, unsigned clk_divide, unsigned clk_freq, TickType_t timeout);

/* Defines how graphics data is written to GDDRAM */
esp_err_t ssd1306_set_memory_address_mode(ssd1306_t ctx, memory_address_mode_t mode, TickType_t timeout);

/* Start and end column used when writing graphics data. Ignored when in page address mode */
esp_err_t ssd1306_set_column_address(ssd1306_t ctx, unsigned start, unsigned end, TickType_t timeout);

/* Start and end page used when writing graphics data. Ignored when in page address mode */
esp_err_t ssd1306_set_page_address(ssd1306_t ctx, unsigned start, unsigned end, TickType_t timeout);

/* Sets Vcomh level */
esp_err_t ssd1306_set_vcomh_deselect_level(ssd1306_t ctx, vcomh_deselect_t level, TickType_t timeout);

/* Display precharge period */
esp_err_t ssd1306_set_precharge_period(ssd1306_t ctx, unsigned phase1, unsigned phase2, TickType_t timeout);

#endif /* COMPONENTS_SSD1306_H_ */
