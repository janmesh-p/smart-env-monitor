
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include "ssd1306.h"

#define I2C_DEVICE "/dev/i2c-2"
#define SSD1306_ADDR 0x3C

static int oled_fd = -1;

typedef struct { char ch; uint8_t col[5]; } glyph5x7_t;
static const glyph5x7_t FONT5x7[] = {
    {'0',{0x3E,0x51,0x49,0x45,0x3E}}, {'1',{0x00,0x42,0x7F,0x40,0x00}},
    {'2',{0x62,0x51,0x49,0x49,0x46}}, {'3',{0x22,0x49,0x49,0x49,0x36}},
    {'4',{0x18,0x14,0x12,0x7F,0x10}}, {'5',{0x2F,0x49,0x49,0x49,0x31}},
    {'6',{0x3E,0x49,0x49,0x49,0x32}}, {'7',{0x01,0x71,0x09,0x05,0x03}},
    {'8',{0x36,0x49,0x49,0x49,0x36}}, {'9',{0x26,0x49,0x49,0x49,0x3E}},
    {'.',{0x00,0x60,0x60,0x00,0x00}}, {'%',{0x63,0x13,0x08,0x64,0x63}},
    {' ',{0x00,0x00,0x00,0x00,0x00}},
    {'T',{0x01,0x01,0x7F,0x01,0x01}}, {'e',{0x32,0x49,0x49,0x49,0x3E}},
    {'m',{0x7E,0x04,0x18,0x04,0x7E}}, {'p',{0x7E,0x09,0x09,0x09,0x06}},
    {'H',{0x7F,0x08,0x08,0x08,0x7F}}, {'u',{0x3E,0x40,0x40,0x20,0x7E}},
    {'i',{0x00,0x00,0x7D,0x00,0x00}}, {'d',{0x06,0x09,0x09,0x09,0x7E}},
    {'y',{0x0E,0x50,0x50,0x50,0x3E}}, {'C',{0x3E,0x41,0x41,0x41,0x22}},
    {'A',{0x7E,0x11,0x11,0x11,0x7E}}, {'U',{0x3F,0x40,0x40,0x40,0x3F}},
    {'O',{0x3E,0x41,0x41,0x41,0x3E}}, {'M',{0x7F,0x02,0x04,0x02,0x7F}},
    {'N',{0x7F,0x02,0x04,0x08,0x7F}}, {'L',{0x7F,0x40,0x40,0x40,0x40}},
};
static const size_t FONT5x7_LEN = sizeof(FONT5x7)/sizeof(FONT5x7[0]);

static const uint8_t* glyph5x7_lookup(char c);
static int oled_cmd(uint8_t cmd);
static int oled_data(const uint8_t *data, int len);

static const uint8_t* glyph5x7_lookup(char c) {
    for (size_t i = 0; i < FONT5x7_LEN; i++)
        if (FONT5x7[i].ch == c) return FONT5x7[i].col;
    return NULL;
}

static int oled_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    return (write(oled_fd, buf, 2) == 2) ? 0 : -1;
}

static int oled_data(const uint8_t *data, int len) {
    if (len <= 0) return 0;
    uint8_t buf[129];
    while (len > 0) {
        int chunk = (len > 128) ? 128 : len;
        buf[0] = 0x40;
        memcpy(buf + 1, data, chunk);
        if (write(oled_fd, buf, chunk + 1) != (chunk + 1)) return -1;
        data += chunk;
        len -= chunk;
    }
    return 0;
}

int ssd1306_init(void) {
    oled_fd = open(I2C_DEVICE, O_RDWR);
    if (oled_fd < 0) { perror("open I2C"); return -1; }
    if (ioctl(oled_fd, I2C_SLAVE, SSD1306_ADDR) < 0) {
        perror("ioctl I2C_SLAVE");
        close(oled_fd);
        oled_fd = -1;
        return -1;
    }
    static const uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    for (size_t i = 0; i < sizeof(init_cmds); i++)
        if (oled_cmd(init_cmds[i]) < 0) return -1;
    ssd1306_clear();
    return 0;
}

void ssd1306_clear(void) {
    uint8_t line[OLED_WIDTH];
    memset(line, 0, sizeof(line));
    for (int page = 0; page < PAGE_COUNT; page++) {
        oled_cmd(0xB0 + page);
        oled_cmd(0x00);
        oled_cmd(0x10);
        oled_data(line, OLED_WIDTH);
    }
}

void ssd1306_draw_string_5x7(const char *s, int page, int start_col) {
    int col = start_col;
    for (; *s && col <= OLED_WIDTH - 6; s++) {
        const uint8_t *g = glyph5x7_lookup(*s);
        if (!g) g = glyph5x7_lookup(' ');
        if (col < 0 || col > OLED_WIDTH - 5 || page < 0 || page >= PAGE_COUNT) continue;
        oled_cmd(0xB0 + page);
        oled_cmd(0x00 + (col & 0x0F));
        oled_cmd(0x10 + ((col >> 4) & 0x0F));
        uint8_t buf[5];
        memcpy(buf, g, 5);
        oled_data(buf, 5);
        col += 6;
    }
}

void ssd1306_draw_big_centered(const char *text) {
    const int h_big = 14;
    const int cw = 10;
    const int spacing = 2;
    int len = (int)strlen(text);
    if (len <= 0) return;
    int width = len * cw + (len - 1) * spacing;
    if (width > OLED_WIDTH) width = OLED_WIDTH;
    uint8_t bmp[16][OLED_WIDTH];
    memset(bmp, 0, sizeof(bmp));
    int x = 0;
    for (int i = 0; i < len; i++) {
        const uint8_t *g = glyph5x7_lookup(text[i]);
        if (!g) g = glyph5x7_lookup(' ');
        for (int col = 0; col < 5; col++) {
            for (int row = 0; row < 7; row++) {
                int on = (g[col] >> row) & 0x01;
                if (on) {
                    int xx = x + col * 2;
                    int yy = row * 2;
                    if (xx + 1 < OLED_WIDTH) {
                        if (yy + 1 < 16) {
                            bmp[yy][xx] = 1;
                            bmp[yy][xx + 1] = 1;
                            bmp[yy + 1][xx] = 1;
                            bmp[yy + 1][xx + 1] = 1;
                        }
                    }
                }
            }
        }
        x += cw + spacing;
        if (x >= OLED_WIDTH) break;
    }
    int start_col = (OLED_WIDTH - width) / 2;
    if (start_col < 0) start_col = 0;
    int start_page = 2;
    uint8_t line[OLED_WIDTH];
    memset(line, 0, sizeof(line));
    for (int col = 0; col < width; col++) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (bmp[bit][col]) byte |= (1u << bit);
        }
        line[start_col + col] = byte;
    }
    oled_cmd(0xB0 + start_page);
    oled_cmd(0x00 + (0 & 0x0F));
    oled_cmd(0x10 + ((0 >> 4) & 0x0F));
    oled_data(line, OLED_WIDTH);
    memset(line, 0, sizeof(line));
    for (int col = 0; col < width; col++) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (bmp[8 + bit][col]) byte |= (1u << bit);
        }
        line[start_col + col] = byte;
    }
    oled_cmd(0xB0 + start_page + 1);
    oled_cmd(0x00 + (0 & 0x0F));
    oled_cmd(0x10 + ((0 >> 4) & 0x0F));
    oled_data(line, OLED_WIDTH);
}

