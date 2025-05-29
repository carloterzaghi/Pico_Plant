#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

// Configurações do I2C
#define I2C_PORT i2c1
#define I2C_ADDR 0x3C  // Endereço do SSD1306

// Configurações do display
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGES (OLED_HEIGHT / 8)

// Comandos SSD1306
#define SSD1306_SET_CONTRAST 0x81
#define SSD1306_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_NORMAL_DISPLAY 0xA6
#define SSD1306_DISPLAY_OFF 0xAE
#define SSD1306_DISPLAY_ON 0xAF
#define SSD1306_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_SET_COM_PINS 0xDA
#define SSD1306_SET_VCOM_DETECT 0xDB
#define SSD1306_SET_DISPLAY_CLOCK_DIV 0xD5
#define SSD1306_SET_PRECHARGE 0xD9
#define SSD1306_SET_MULTIPLEX 0xA8
#define SSD1306_MEMORY_MODE 0x20
#define SSD1306_COLUMN_ADDR 0x21
#define SSD1306_PAGE_ADDR 0x22
#define SSD1306_COM_SCAN_DEC 0xC8
#define SSD1306_SEG_REMAP 0xA0
#define SSD1306_CHARGE_PUMP 0x8D
#define SSD1306_SET_START_LINE 0x40

// Funções básicas do OLED
void oled_send_cmd(uint8_t cmd);
void oled_send_data(uint8_t *data, size_t len);
void oled_init(void);
void oled_clear(void);
void oled_update(void);

// Funções de desenho
void oled_set_pixel(int x, int y, bool on);
void oled_draw_circle(int cx, int cy, int radius, bool filled, bool on);
void oled_draw_line(int x0, int y0, int x1, int y1, bool on);
void oled_draw_ellipse(int cx, int cy, int rx, int ry, bool filled, bool on);
void oled_draw_arc(int cx, int cy, int radius, int start_angle, int end_angle, bool on);

// Funções para desenhar faces
void draw_happy_face(void);
void draw_sad_face(void);

#endif // OLED_SSD1306_H