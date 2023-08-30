#include "main.h"

#define ALARM_PROGRAM 1

void app_main(void)
{
    /* Initialize devices */
    int ret;
    ret = init_devices();
    if (ret != 0)
    {
        printf("unable to initialize devices");
        return;
    }

    /* SNTP initialize*/
    SNTP_initialize();
    display_clear();

    /* Initialize alarm*/
    state_table_init(&alarm_clock);
    alarm_init(&alarm_clock);

    /* Start the clock*/
    uint32_t last_tick = system_uptime();
    tick_event_t te;

    // mqtt_app_start();

    alarm_time_t alarm1 = {10, 30, 22};
    alarm_time_t alarm2 = {15, 45, 9};
    alarm_time_t alarm3 = {0, 2, 18};
    alarm_time_t alarm4 = {0, 0, 18};


    // insert(&alarm_clock.alarm_list, alarm1);
    // insert(&alarm_clock.alarm_list, alarm2);
    insert(&alarm_clock.alarm_list, alarm3);
    insert(&alarm_clock.alarm_list, alarm4);

    printf("Alarm Times: ");
    printList(alarm_clock.alarm_list);


#if ALARM_PROGRAM
    while (1)
    {
        // Wait for incoming events on the event queue.
        rotary_encoder_event_t event = {0};
        if (xQueueReceive(rotary_event_queue, &event, 10 / portTICK_PERIOD_MS) == pdTRUE)
        {
            ESP_LOGI("Rotary encoder", "Event: position %ld, direction %s", event.state.position,
                     event.state.direction ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
            te.super.sig = SCROLL;
            if(event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE){
                if(alarm_clock.selected_item < MENU_VIEW_ALARMS) alarm_clock.selected_item++;
            } else {
                if(alarm_clock.selected_item >= MENU_DELETE_ALARMS) alarm_clock.selected_item--;
            }
            event_dispatcher(&alarm_clock, &te.super);
        }
        /* Dispatch a tick event every second */
        if (system_uptime() - last_tick >= MILLIS_PER_SECOND)
        {
            // 100ms has passed; millis_per_second for testing quickly
            last_tick = system_uptime();
            te.super.sig = TIME_TICK;
            if (++te.ss > 10)
            {
                te.ss = 1;
            }
            event_dispatcher(&alarm_clock, &te.super);
        }

        /* If the Snooze button ISR sets the flag to 1, this code block will run */
        if (button_flag == 1)
        {
            te.super.sig = BUTTON;
            button_prev_millis = button_current_millis;

            // Release the mutex to allow the ISR to give it again
            button_flag = 0;
            if (alarm_clock.active_state == ALARM)
            {
                alarm_time_t next_alarm = {.second = 0, .minute = 1, .hour = 0};
                alarm_clock.remaining_time = next_alarm;
            }

            event_dispatcher(&alarm_clock, &te.super);

        }
    }
#endif
    ESP_LOGE(TAG, "queue receive failed");

    ESP_ERROR_CHECK(rotary_encoder_uninit(&rotary_info));

    freeList(alarm_clock.alarm_list);
}

/* Snooze ISR */
static void IRAM_ATTR press_button(void *arg)
{
    button_current_millis = system_uptime();
    if ((button_current_millis - button_prev_millis > DEBOUNCE_DELAY_MS))
    {
        button_flag = 1;
    }
}

/* Device initialization function*/
static int init_devices()
{
    /* Configure the snooze button */
    gpio_config_t snooze_button_conf;

    snooze_button_conf.intr_type = GPIO_INTR_NEGEDGE;
    snooze_button_conf.pin_bit_mask = 1ULL << BUTTON_GPIO_PIN;
    snooze_button_conf.mode = GPIO_MODE_INPUT;
    snooze_button_conf.pull_up_en = 1;
    snooze_button_conf.pull_down_en = 0;
    gpio_config(&snooze_button_conf);

    /* Configure interrupt for the snooze button */
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_PIN, press_button, NULL);

    /* Configure SSD1306 128x64 I2C display */
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);

    /* Configure Wi-Fi */
    esp_err_t ret = nvs_flash_init(); // Initialize NVS
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // Initialise the rotary encoder device with the GPIOs for A and B signals
    ESP_ERROR_CHECK(rotary_encoder_init(&rotary_info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&rotary_info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&rotary_info));
#endif

    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    rotary_event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&rotary_info, rotary_event_queue));
    return 0;
}

/* State table initialization function*/
static void state_table_init(alarm_t *const mobj)
{
    static e_handler_t state_table[MAX_STATES][MAX_SIGNALS] = {
        [IDLE] = {IDLE_TIME_TICK, IDLE_BUTTON, NULL, IDLE_ENTRY, IDLE_EXIT},
        [ALARM] = {ALARM_TIME_TICK, ALARM_BUTTON, NULL, ALARM_ENTRY, ALARM_EXIT},
        [MENU] = {MENU_TIME_TICK, MENU_BUTTON, MENU_SCROLL, MENU_ENTRY, MENU_EXIT},
        //[SET_ALARM] = {SET_ALARM_TIME_TICK, SET_ALARM_BUTTON, NULL, SET_ALARM_ENTRY, SET_ALARM_EXIT},
    };

    mobj->state_table = (uintptr_t *)&state_table[0][0];
}

/* This function dispatches events as per the state table */
static void event_dispatcher(alarm_t *const mobj, event_t const *const e)
{
    event_status_t status = EVENT_IGNORED;
    state_t source, target;
    e_handler_t ehandler;

    source = mobj->active_state;
    ehandler = (e_handler_t)mobj->state_table[mobj->active_state * MAX_SIGNALS + e->sig];
    if (ehandler)
    {
        status = (*ehandler)(mobj, e);
    }

    if (status == EVENT_TRANSITION)
    {
        target = mobj->active_state;
        event_t ee;
        // 1. run the exit action for the source state
        ee.sig = EXIT;
        ehandler = (e_handler_t)mobj->state_table[source * MAX_SIGNALS + EXIT];
        if (ehandler)
        {
            (*ehandler)(mobj, &ee);
        }

        // 2. run the entry action for the target state
        ee.sig = ENTRY;
        ehandler = (e_handler_t)mobj->state_table[target * MAX_SIGNALS + ENTRY];
        if (ehandler)
        {
            (*ehandler)(mobj, &ee);
        }
    }
}

/* Provides the system uptime in milliseconds
    Arguments: none
    Return type: int32_t
*/
int32_t system_uptime(void)
{
    int64_t uptimeMicros = esp_timer_get_time();

    // Convert uptime to milliseconds
    int32_t uptimeMillis = uptimeMicros / 1000;
    return uptimeMillis;
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

static void obtain_time(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "time.google.com"); // Replace with your preferred NTP server
    esp_sntp_init();

    ESP_LOGI(TAG, "SERVER_SET");

    // Wait for the system time to be set
    time_t now = 0;
    struct tm timeinfo;
    int retry = 0;
    const int retry_count = SNTP_MAXIMUM_RETRY;
    ESP_LOGI(TAG, "Entering SNTP time set loop");
    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        print_message("SNTP init", 10, 3);
        vTaskDelay(pdMS_TO_TICKS(1000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGE(TAG, "Failed to get network time.");
    } else {
        ESP_LOGI(TAG, "Network time obtained successfully.");
    }
}

void SNTP_initialize(void)
{
    obtain_time();

    // Set timezone to Indian Standard Time
    setenv("TZ", "IST-5:30", 1);
    tzset();
    time(&now);
    localtime_r(&now, &alarm_clock.ntp_time);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &alarm_clock.ntp_time);
    ESP_LOGI(TAG, "The current date/time in Kolkata is: %s", strftime_buf);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "hello", "bunda maane", 0, 2, 0);
        ESP_LOGI(MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "hello", 0);
        ESP_LOGI(MQTT_TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "hello", "subscribed to you", 0, 2, 0);
        ESP_LOGI(MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(MQTT_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(MQTT_TAG, "Last error %s: 0x%x", message, error_code);
    }
}