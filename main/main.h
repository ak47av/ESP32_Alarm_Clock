#include "esp_timer.h"
#include <time.h>
#include "oled.h"
#include "state_machine.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include "rotary_encoder.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "mqtt_client.h"

#include "lwip/err.h"
#include "lwip/sys.h"

////////////////////////// DEFINES ////////////////////
#define MILLIS_PER_SECOND 100
#define DEBOUNCE_DELAY_MS 200

#define BUTTON_GPIO_PIN     GPIO_NUM_10 // Replace with the GPIO pin number you're using
#define BUTTON_GPIO_PULL    GPIO_PULLUP_ONLY // Use GPIO_PULLDOWN_ONLY for active low buttons
#define BUTTON_INTERRUPT_EDGE   GPIO_INTR_NEGEDGE // Use GPIO_INTR_NEGEDGE for falling edge

#define LED_GPIO_PIN     GPIO_NUM_2 // Replace with the GPIO pin number you're using

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define EXAMPLE_ESP_WIFI_SSID      "ARUN_ACT"
#define EXAMPLE_ESP_WIFI_PASS      "password"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10

// SNTP defines
#define SNTP_MAXIMUM_RETRY 15

// ROTARY ENCODER DEFINES
#define ROT_ENC_A_GPIO 8
#define ROT_ENC_B_GPIO 9

#define ENABLE_HALF_STEPS false  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          0      // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION    false  // Set to true to reverse the clockwise/counterclockwise sense
////////////////////////// DEFINES ////////////////////

///////////////////////////// VARIABLES /////////////////////////
alarm_t alarm_clock;
char menu_items[MENU_MAX_ITEMS][20] = {
    "Set a new alarm",
    "Delete an alarm",
    "View all alarms"
};

int32_t button_current_millis = 0;
int32_t button_prev_millis = 0;
int button_flag = 0;

SSD1306_t dev;
   
time_t now;
char strftime_buf[64];

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "wifi station";
static const char *MQTT_TAG = "MQTT";

static const char *MQTT_BROKER_URI = "mqtt://192.168.0.101:1883" ;

static int s_retry_num = 0; // Counting the number of retries when connecting to Wi-Fi

QueueHandle_t rotary_event_queue;
rotary_encoder_info_t rotary_info = { 0 };
///////////////////////////// VARIABLES /////////////////////////

///////////////////////////// FUNCTIONS /////////////////////////

static int init_devices();
static void event_dispatcher(alarm_t *const mobj, event_t const *const e);
static void state_table_init(alarm_t *const mobj);
static int32_t system_uptime(void);
static void press_button(void *arg);

// static void menu_init(alarm_t *const mobj);

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_sta(void);

static void obtain_time(void);
void SNTP_initialize(void);

static void log_error_if_nonzero(const char *message, int error_code);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_app_start(void);
///////////////////////////// FUNCTIONS /////////////////////////