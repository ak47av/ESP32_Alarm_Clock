#include <stdint.h>
#include "oled.h"

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
} alarm_time_t;

typedef struct alarmNode{
    alarm_time_t alarm;
    struct alarmNode *nextAlarm;
}alarmNode;

void insert(alarmNode **head, alarm_time_t alarm);
int compareTimes(alarm_time_t time1, alarm_time_t time2); 
void printList(alarmNode *head);
void freeList(alarmNode *head);
void removeFirst(alarmNode **head);

void increase_time(alarm_time_t *time);
void decrease_time(alarm_time_t *time);
void display_time(alarm_time_t *time);
int is_zero(alarm_time_t *time);
int is_equal(alarm_time_t time1, alarm_time_t time2);

alarm_time_t timeDifference(alarm_time_t time1, alarm_time_t time2);