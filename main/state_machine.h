#include "stdint.h"
#include "alarm_time.h"
#include <time.h>
#include <stdio.h>


/* Signal type */
typedef enum {
	TIME_TICK,
	BUTTON,
	SCROLL,
	/* Internal activity signals */
	ENTRY,
	EXIT,
	MAX_SIGNALS
} signal_t;

/* State type */
typedef enum {
	IDLE,
	ALARM,
	MENU,
	SET_ALARM,
	MAX_STATES
} state_t;

/*Generic(Super) event structure */
typedef struct {
	uint8_t sig;
} event_t;

/* For user generated events */
typedef struct {
	event_t super;
} user_event_t;

/* For tick event */
typedef struct {
	event_t super;
	uint8_t ss;
} tick_event_t;

/* Event status type */
typedef enum {
	EVENT_HANDLED,
	EVENT_IGNORED,
	EVENT_TRANSITION
} event_status_t;

typedef enum {
	MENU_SET_ALARM,
	MENU_DELETE_ALARMS,
	MENU_VIEW_ALARMS,
	MENU_MAX_ITEMS
} menu_item_t;

extern char menu_items[MENU_MAX_ITEMS][20];

/* Main application structure */
typedef struct {
	struct tm ntp_time;
	alarm_time_t curr_time;
	alarm_time_t remaining_time;
	state_t active_state;
	uintptr_t *state_table;
	menu_item_t selected_item;	
} alarm_t;

extern SSD1306_t dev;

/* Alarm init */
void alarm_init(alarm_t *mobj);

/* Event handler type */
typedef event_status_t (*e_handler_t)(alarm_t *const mobj, event_t const *const e);

//////////////////////////// EVENT HANDLERS ///////////////////////////////

void alarm_init(alarm_t *mobj);

/* Event handlers for IDLE State */

event_status_t IDLE_ENTRY(alarm_t *const mobj, event_t const *const e);     // ENTRY handler
event_status_t IDLE_EXIT(alarm_t *const mobj, event_t const *const e);      // EXIT handler
event_status_t IDLE_TIME_TICK(alarm_t *const mobj, event_t const *const e); // TIME_TICK handler
event_status_t IDLE_BUTTON (alarm_t *const mobj, event_t const *const e);   // BUTTON Handler

/* Event handlers for ALARM State */

event_status_t ALARM_ENTRY(alarm_t *const mobj, event_t const *const e);  // ENTRY handler
event_status_t ALARM_EXIT(alarm_t *const mobj, event_t const *const e);   // EXIT handler
event_status_t ALARM_TIME_TICK(alarm_t *const mobj, event_t const *const e); // TIME_TICK handler
event_status_t ALARM_BUTTON(alarm_t *const mobj, event_t const *const e); // BUTTON handler 

/* Event handlers for MENU State */
event_status_t MENU_ENTRY(alarm_t *const mobj, event_t const *const e);  // ENTRY handler
event_status_t MENU_EXIT(alarm_t *const mobj, event_t const *const e);   // EXIT handler
event_status_t MENU_TIME_TICK(alarm_t *const mobj, event_t const *const e); // TIME_TICK Handler
event_status_t MENU_BUTTON(alarm_t *const mobj, event_t const *const e); // TIME_TICK handler
event_status_t MENU_SCROLL(alarm_t *const mobj, event_t const *const e)

/* Event handlers for MENU State */
event_status_t SET_ALARM_ENTRY(alarm_t *const mobj, event_t const *const e);  // ENTRY handler
event_status_t SET_ALARM_EXIT(alarm_t *const mobj, event_t const *const e);   // EXIT handler
event_status_t SET_ALARM_TIME_TICK(alarm_t *const mobj, event_t const *const e); // TIME_TICK Handler
event_status_t SET_ALARM_BUTTON(alarm_t *const mobj, event_t const *const e); // TIME_TICK handler
event_status_t SET_ALARM_SCROLL(alarm_t *const mobj, event_t const *const e)
//////////////////////////// EVENT HANDLERS ///////////////////////////////