#include "oled_ssd1306.h"
#include "hardware/i2c.h"
#include <string.h> 
#include <stdlib.h>   
#include <math.h>


// Buffer do display
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];

// Função para enviar comando
void oled_send_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_blocking(I2C_PORT, I2C_ADDR, buf, 2, false);
}

// Função para enviar dados
void oled_send_data(uint8_t *data, size_t len) {
    uint8_t buf[len + 1];
    buf[0] = 0x40;  // Data mode
    memcpy(buf + 1, data, len);
    i2c_write_blocking(I2C_PORT, I2C_ADDR, buf, len + 1, false);
}

// Inicializar o display
void oled_init() {
    oled_send_cmd(SSD1306_DISPLAY_OFF);
    oled_send_cmd(SSD1306_SET_DISPLAY_CLOCK_DIV);
    oled_send_cmd(0x80);
    oled_send_cmd(SSD1306_SET_MULTIPLEX);
    oled_send_cmd(OLED_HEIGHT - 1);
    oled_send_cmd(SSD1306_SET_DISPLAY_OFFSET);
    oled_send_cmd(0x00);
    oled_send_cmd(SSD1306_SET_START_LINE | 0x00);
    oled_send_cmd(SSD1306_CHARGE_PUMP);
    oled_send_cmd(0x14);
    oled_send_cmd(SSD1306_MEMORY_MODE);
    oled_send_cmd(0x00);
    oled_send_cmd(SSD1306_SEG_REMAP | 0x01);
    oled_send_cmd(SSD1306_COM_SCAN_DEC);
    oled_send_cmd(SSD1306_SET_COM_PINS);
    oled_send_cmd(0x12);
    oled_send_cmd(SSD1306_SET_CONTRAST);
    oled_send_cmd(0xCF);
    oled_send_cmd(SSD1306_SET_PRECHARGE);
    oled_send_cmd(0xF1);
    oled_send_cmd(SSD1306_SET_VCOM_DETECT);
    oled_send_cmd(0x40);
    oled_send_cmd(SSD1306_DISPLAY_ALL_ON_RESUME);
    oled_send_cmd(SSD1306_NORMAL_DISPLAY);
    oled_send_cmd(SSD1306_DISPLAY_ON);
}

// Limpar o display
void oled_clear() {
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

// Atualizar o display
void oled_update() {
    oled_send_cmd(SSD1306_COLUMN_ADDR);
    oled_send_cmd(0);
    oled_send_cmd(OLED_WIDTH - 1);
    oled_send_cmd(SSD1306_PAGE_ADDR);
    oled_send_cmd(0);
    oled_send_cmd(OLED_PAGES - 1);
    
    oled_send_data(oled_buffer, sizeof(oled_buffer));
}

// Definir um pixel
void oled_set_pixel(int x, int y, bool on) {
    if (x >= 0 && x < OLED_WIDTH && y >= 0 && y < OLED_HEIGHT) {
        if (on) {
            oled_buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
        } else {
            oled_buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
        }
    }
}

// Desenhar círculo (algoritmo de Bresenham)
void oled_draw_circle(int cx, int cy, int radius, bool filled, bool on) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (x <= y) {
        if (filled) {
            // Desenhar linhas horizontais para preencher
            for (int i = cx - x; i <= cx + x; i++) {
                oled_set_pixel(i, cy + y, on);
                oled_set_pixel(i, cy - y, on);
            }
            for (int i = cx - y; i <= cx + y; i++) {
                oled_set_pixel(i, cy + x, on);
                oled_set_pixel(i, cy - x, on);
            }
        } else {
            // Desenhar apenas o contorno
            oled_set_pixel(cx + x, cy + y, on);
            oled_set_pixel(cx - x, cy + y, on);
            oled_set_pixel(cx + x, cy - y, on);
            oled_set_pixel(cx - x, cy - y, on);
            oled_set_pixel(cx + y, cy + x, on);
            oled_set_pixel(cx - y, cy + x, on);
            oled_set_pixel(cx + y, cy - x, on);
            oled_set_pixel(cx - y, cy - x, on);
        }
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// Desenhar uma linha
void oled_draw_line(int x0, int y0, int x1, int y1, bool on) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        oled_set_pixel(x0, y0, on);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Desenhar elipse (para olhos e boca)
void oled_draw_ellipse(int cx, int cy, int rx, int ry, bool filled, bool on) {
    int x = 0;
    int y = ry;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int tworx2 = 2 * rx2;
    int twory2 = 2 * ry2;
    int p1 = ry2 - rx2 * ry + rx2 / 4;
    int dx = 0;
    int dy = tworx2 * y;
    
    // Região 1
    while (dx < dy) {
        if (filled) {
            for (int i = cx - x; i <= cx + x; i++) {
                oled_set_pixel(i, cy + y, on);
                oled_set_pixel(i, cy - y, on);
            }
        } else {
            oled_set_pixel(cx + x, cy + y, on);
            oled_set_pixel(cx - x, cy + y, on);
            oled_set_pixel(cx + x, cy - y, on);
            oled_set_pixel(cx - x, cy - y, on);
        }
        
        if (p1 < 0) {
            x++;
            dx += twory2;
            p1 += dx + ry2;
        } else {
            x++;
            y--;
            dx += twory2;
            dy -= tworx2;
            p1 += dx - dy + ry2;
        }
    }
    
    // Região 2
    int p2 = ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y >= 0) {
        if (filled) {
            for (int i = cx - x; i <= cx + x; i++) {
                oled_set_pixel(i, cy + y, on);
                oled_set_pixel(i, cy - y, on);
            }
        } else {
            oled_set_pixel(cx + x, cy + y, on);
            oled_set_pixel(cx - x, cy + y, on);
            oled_set_pixel(cx + x, cy - y, on);
            oled_set_pixel(cx - x, cy - y, on);
        }
        
        if (p2 > 0) {
            y--;
            dy -= tworx2;
            p2 += rx2 - dy;
        } else {
            y--;
            x++;
            dx += twory2;
            dy -= tworx2;
            p2 += dx - dy + rx2;
        }
    }
}

// Desenhar arco para sorriso/tristeza
void oled_draw_arc(int cx, int cy, int radius, int start_angle, int end_angle, bool on) {
    for (int angle = start_angle; angle <= end_angle; angle++) {
        float rad = angle * M_PI / 180.0;
        int x = cx + (int)(radius * cos(rad));
        int y = cy + (int)(radius * sin(rad));
        oled_set_pixel(x, y, on);
        
        // Tornar a linha mais espessa
        oled_set_pixel(x + 1, y, on);
        oled_set_pixel(x, y + 1, on);
        oled_set_pixel(x + 1, y + 1, on);
    }
}

// Desenhar rosto feliz
void draw_happy_face() {
    oled_clear();
    
    // Rosto (círculo grande)
    oled_draw_circle(64, 32, 28, false, true);
    
    // Olhos (círculos preenchidos)
    oled_draw_circle(54, 24, 4, true, true);  // Olho esquerdo
    oled_draw_circle(74, 24, 4, true, true);  // Olho direito
    
    // Sorriso (arco de 20 a 160 graus)
    oled_draw_arc(64, 32, 15, 20, 160, true);
    
    // Bochechas (pequenos círculos)
    oled_draw_circle(42, 35, 3, false, true);  // Bochecha esquerda
    oled_draw_circle(86, 35, 3, false, true);  // Bochecha direita
    
    oled_update();
}

// Desenhar rosto triste
void draw_sad_face() {
    oled_clear();
    
    // Rosto (círculo grande)
    oled_draw_circle(64, 32, 28, false, true);
    
    // Olhos tristes (elipses inclinadas)
    oled_draw_ellipse(54, 24, 3, 5, true, true);  // Olho esquerdo
    oled_draw_ellipse(74, 24, 3, 5, true, true);  // Olho direito
    
    // Sobrancelhas tristes
    oled_draw_line(48, 18, 58, 20, true);  // Sobrancelha esquerda
    oled_draw_line(70, 20, 80, 18, true);  // Sobrancelha direita
    
    // Boca triste (arco invertido de 200 a 340 graus)
    oled_draw_arc(64, 48, 12, 200, 340, true);
    
    // Lágrimas
    oled_draw_line(50, 28, 48, 35, true);  // Lágrima esquerda
    oled_draw_line(48, 35, 46, 38, true);
    oled_draw_circle(46, 39, 2, true, true);  // Gota
    
    oled_update();
}