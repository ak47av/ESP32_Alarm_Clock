#include "state_machine.h"
#include <stdlib.h>

void alarm_init(alarm_t *mobj){
    event_t ee;
    e_handler_t ehandler;
    ee.sig = ENTRY;
    mobj->active_state = IDLE;

    alarm_time_t zero_time = {.second=0, .minute=0, .hour=0};
    alarm_time_t alarm_time = {.second=20, .minute=0, .hour=0};
    mobj->curr_time = zero_time;
    mobj->remaining_time = alarm_time; 
    ehandler = (e_handler_t) mobj->state_table[IDLE * MAX_SIGNALS + ENTRY];
    (*ehandler)(mobj,&ee);
}

event_status_t IDLE_ENTRY(alarm_t *const mobj, event_t const *const e)     
{
    printf("IDLE_ENTRY\n");
    display_time(&mobj->curr_time);

    char *alarm_buf = calloc(20, sizeof(char));
    sprintf(alarm_buf, "Alarm: %02d:%02d:%02d", mobj->remaining_time.hour, mobj->remaining_time.minute, mobj->remaining_time.second);
    print_message(alarm_buf, 24, 3);
    free(alarm_buf);
    return EVENT_HANDLED;
}

event_status_t IDLE_EXIT(alarm_t *const mobj, event_t const *const e)
{
    printf("IDLE_EXIT\n");
    display_clear();
    return EVENT_HANDLED;
}

event_status_t IDLE_TIME_TICK(alarm_t *const mobj, event_t const *const e)
{
    if( ((tick_event_t *)(e))->ss == 10){
        increase_time(&mobj->curr_time);
        decrease_time(&mobj->remaining_time);
        if(is_zero(&mobj->remaining_time))
        {
            mobj->active_state = ALARM;
            return EVENT_TRANSITION;
        }
        display_time(&mobj->curr_time);
        char *alarm_buf = calloc(20, sizeof(char));
        sprintf(alarm_buf, "Alarm: %02d:%02d:%02d", mobj->remaining_time.hour, mobj->remaining_time.minute, mobj->remaining_time.second);
        print_message(alarm_buf, 24, 3);
        free(alarm_buf);
        return EVENT_HANDLED;
    }
    return EVENT_IGNORED;
}

event_status_t ALARM_ENTRY(alarm_t *const mobj, event_t const *const e)
{
    printf("ALARM_ENTRY\n");
    display_time(&mobj->curr_time);
    return EVENT_HANDLED;
}  

event_status_t ALARM_EXIT(alarm_t *const mobj, event_t const *const e)
{
    printf("ALARM_EXIT\n");
    display_clear();
    return EVENT_HANDLED;
}

event_status_t ALARM_TIME_TICK(alarm_t *const mobj, event_t const *const e)
{
    if( ((tick_event_t *)(e))->ss == 10){
        increase_time(&mobj->curr_time);
        display_time(&mobj->curr_time);
        print_message("Hit snooze", 10, 3);
       // display_time(&mobj->remaining_time, 0, 42);
        return EVENT_HANDLED;
    }
    return EVENT_IGNORED;
}

event_status_t ALARM_SNOOZE(alarm_t *const mobj, event_t const *const e) 
{
    printf("ALARM_SNOOZE\n");
    mobj->active_state = IDLE;
    return EVENT_TRANSITION;
}