#include <stdint.h>
#include "oled.h"

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
} alarm_time_t;

void increase_time(alarm_time_t *time);
void decrease_time(alarm_time_t *time);
void display_time(alarm_time_t *time);
int is_zero(alarm_time_t *time);