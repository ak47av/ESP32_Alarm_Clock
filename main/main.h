#include "esp_timer.h"
#include "oled.h"
#include "state_machine.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "ssd1306.h"

////////////////////////// DEFINES ////////////////////

#define MILLIS_PER_SECOND 100
#define DEBOUNCE_DELAY_MS 100

#define BUTTON_GPIO_PIN     GPIO_NUM_3 // Replace with the GPIO pin number you're using
#define BUTTON_GPIO_PULL    GPIO_PULLUP_ONLY // Use GPIO_PULLDOWN_ONLY for active low buttons
#define BUTTON_INTERRUPT_EDGE   GPIO_INTR_NEGEDGE // Use GPIO_INTR_NEGEDGE for falling edge

#define LED_GPIO_PIN     GPIO_NUM_2 // Replace with the GPIO pin number you're using
////////////////////////// DEFINES ////////////////////

///////////////////////////// VARIABLES /////////////////////////

static alarm_t alarm;

int32_t snooze_current_millis = 0;
int32_t snooze_prev_millis = 0;

int snooze_flag = 0;
SSD1306_t dev;

///////////////////////////// VARIABLES /////////////////////////

///////////////////////////// FUNCTIONS /////////////////////////

static int init_devices();
static void event_dispatcher(alarm_t *const mobj, event_t const *const e);
static void state_table_init(alarm_t *const mobj);
static int32_t system_uptime(void);
static void hit_snooze(void *arg);

///////////////////////////// FUNCTIONS /////////////////////////