#include "stdint.h"
#include "alarm_time.h"
#include <time.h>
#include <stdio.h>

#define MAX_STATES 2

/* Signal type */
typedef enum {
	SNOOZE,
	TIME_TICK,
	/* Internal activity signals */
	ENTRY,
	EXIT,
	MAX_SIGNALS
} signal_t;

/* State type */
typedef enum {
	IDLE,
	ALARM,
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

/* Main application structure */
typedef struct {
	struct tm ntp_time;
	alarm_time_t curr_time;
	alarm_time_t remaining_time;
	state_t active_state;
	uintptr_t *state_table;
} alarm_t;

/* Alarm init */
void alarm_init(alarm_t *mobj);

/* Event handler type */
typedef event_status_t (*e_handler_t)(alarm_t *const mobj, event_t const *const e);

//////////////////////////// EVENT HANDLERS ///////////////////////////////

void alarm_init(alarm_t *mobj);

/* Event handlers for IDLE State*/

event_status_t IDLE_ENTRY(alarm_t *const mobj, event_t const *const e);     // ENTRY handler
event_status_t IDLE_EXIT(alarm_t *const mobj, event_t const *const e);      // EXIT handler
event_status_t IDLE_TIME_TICK(alarm_t *const mobj, event_t const *const e); // TIME_TICK handler

/* Event handlers for ALARM State*/

event_status_t ALARM_ENTRY(alarm_t *const mobj, event_t const *const e);  // ENTRY handler
event_status_t ALARM_EXIT(alarm_t *const mobj, event_t const *const e);   // EXIT handler
event_status_t ALARM_TIME_TICK(alarm_t *const mobj, event_t const *const e);
event_status_t ALARM_SNOOZE(alarm_t *const mobj, event_t const *const e); // TIME_TICK handler

//////////////////////////// EVENT HANDLERS ///////////////////////////////