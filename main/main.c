#include "main.h"

#define ALARM 1

void app_main(void)
{
    int ret;

    ret = init_devices();
    if (ret != 0)
    {
        printf("unable to initialize devices");
        return;
    }

    state_table_init(&alarm);
    alarm_init(&alarm);

    uint32_t last_tick = system_uptime();

    tick_event_t te;
    while (1)
    {
#if ALARM
        if (system_uptime() - last_tick >= MILLIS_PER_SECOND) {
        	// 100ms has passed; millis_per_second for testing quickly
        	last_tick = system_uptime();
        	te.super.sig = TIME_TICK;
        	if (++te.ss > 10) {
        		te.ss = 1;
        	}
        	event_dispatcher(&alarm, &te.super);
        }
        if (snooze_flag == 1) {
           // Perform the lengthy operation here
           te.super.sig = SNOOZE;
           snooze_prev_millis = snooze_current_millis;
           
           // Release the mutex to allow the ISR to give it again
           snooze_flag = 0;
           alarm_time_t next_alarm = {.second=0, .minute=1, .hour=0};
           alarm.remaining_time = next_alarm; 

           event_dispatcher(&alarm, &te.super);
        }
#endif
    }
}

static void IRAM_ATTR hit_snooze(void *arg)
{
    snooze_current_millis = system_uptime();
    if ((snooze_current_millis - snooze_prev_millis > DEBOUNCE_DELAY_MS))
    {
        snooze_flag = 1;
    }
}

static int init_devices()
{
    gpio_config_t snooze_button_conf;

    snooze_button_conf.intr_type = GPIO_INTR_NEGEDGE;
    snooze_button_conf.pin_bit_mask = 1ULL << BUTTON_GPIO_PIN;
    snooze_button_conf.mode = GPIO_MODE_INPUT;
    snooze_button_conf.pull_up_en = 1;
    snooze_button_conf.pull_down_en = 0;
    gpio_config(&snooze_button_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_PIN, hit_snooze, NULL);

    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);

    return 0;
}

static void state_table_init(alarm_t *const mobj)
{
    static e_handler_t state_table[MAX_STATES][MAX_SIGNALS] = {
        [IDLE] = {NULL, IDLE_TIME_TICK, IDLE_ENTRY, IDLE_EXIT},
        [ALARM] = {ALARM_SNOOZE, ALARM_TIME_TICK, ALARM_ENTRY, ALARM_EXIT},
    };

    mobj->state_table = (uintptr_t *)&state_table[0][0];
}

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