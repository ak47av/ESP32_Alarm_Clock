
#include "ssd1306.h"
#include "font8x8_basic.h"

///////////////////////////// VARIABLES /////////////////////////

extern SSD1306_t dev;

///////////////////////////// VARIABLES /////////////////////////


///////////////////////////// FUNCTIONS /////////////////////////

int print_message(char *str, int strlen, int page);
int print_message_x3(char *str, int strlen, int page);
int display_clear();

///////////////////////////// FUNCTIONS /////////////////////////