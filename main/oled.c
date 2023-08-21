#include <stdio.h>
#include "oled.h"

#define tag "SSD1306"
#define OLED_DEBUG 1

/*print_message(char *str, int strlen, int page)*/
int print_message(char *str, int strlen, int page)
{
#if OLED_DEBUG
	ssd1306_display_text(&dev, page, str, strlen, false);
#endif
    return 0;
}

int print_message_x3(char *str, int strlen, int page)
{
#if OLED_DEBUG
	ssd1306_display_text_x3(&dev, page, str, strlen, false);
#endif
    return 0;
}

int display_clear(void)
{
#if OLED_DEBUG
    ssd1306_clear_screen(&dev, false);
#endif
    return 0;
}