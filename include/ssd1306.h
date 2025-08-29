#ifndef SSD1306_H
#define SSD1306_H

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define PAGE_COUNT 8

int ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_draw_string_5x7(const char *s, int page, int start_col);
void ssd1306_draw_big_centered(const char *text);

#endif // SSD1306_H
