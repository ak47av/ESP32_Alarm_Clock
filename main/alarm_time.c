#include "alarm_time.h"
#include <stdio.h> 

/* This function increases the clock timer */
void increase_time(alarm_time_t *time)
{
	time->second++;
	if (time->second == 60) {
		time->minute++;
		time->second = 0;
		if (time->minute == 60) {
			time->hour++;
			time->minute = 0;
			if (time->hour == 24) {
				time->hour = 0;
			}
		}
	}
}

/* This function decreases the alarm timer */
void decrease_time(alarm_time_t *time)
{
	if ((time->hour == 0) && (time->minute == 0) && (time->second == 0)) {
		return;
	}

	if (time->second == 0) {
		time->second = 59;
		if (time->minute == 0) {
			time->minute = 59;
			if (time->hour != 0) {
				time->hour--;
			}
		} else {
			time->minute--;
		}
	} else {
		time->second--;
	}
}

void display_time(alarm_time_t *time)
{
    static char msg_buf[15];
    sprintf(msg_buf, "%02d:%02d\n", time->hour, time->minute);
	print_message_x3(msg_buf, 5, 0);
}

int is_zero(alarm_time_t *time)
{
    if((time->second==0) && (time->minute==0) && (time->hour==0))
        return 1;
    else return 0;
}

int is_equal(alarm_time_t time1, alarm_time_t time2)
{
   if (time1.hour != time2.hour) 
        return -1; 
    if (time1.minute != time2.minute) 
        return -1;
    if(time1.second != time2.second)
        return -1;
    return 0; 
}

void insert(alarmNode **head, alarm_time_t alarm) {
    alarmNode *newalarmNode = (alarmNode*)malloc(sizeof(alarmNode));
    newalarmNode->alarm = alarm;
    newalarmNode->nextAlarm = NULL;

    if (*head == NULL || compareTimes(alarm, (*head)->alarm) < 0) {
        newalarmNode->nextAlarm = *head;
        *head = newalarmNode;
    } else {
        alarmNode *current = *head;
        while (current->nextAlarm != NULL && compareTimes(alarm, current->nextAlarm->alarm) >= 0) {
            current = current->nextAlarm;
        }
        newalarmNode->nextAlarm = current->nextAlarm;
        current->nextAlarm = newalarmNode;
    }
}

int compareTimes(alarm_time_t time1, alarm_time_t time2) {
    if (time1.hour != time2.hour) {
        return time1.hour - time2.hour;
    } else if (time1.minute != time2.minute) {
        return time1.minute - time2.minute;
    } else {
        return time1.second - time2.second;
    }
}

void printList(alarmNode *head) {
    alarmNode *current = head;
    while (current != NULL) {
        printf("%02d:%02d:%02d -> ", current->alarm.hour, current->alarm.minute, current->alarm.second);
        current = current->nextAlarm;
    }
    printf("NULL\n");
}

void freeList(alarmNode *head) {
    alarmNode *current = head;
    while (current != NULL) {
        alarmNode *temp = current;
        current = current->nextAlarm;
        free(temp);
    }
} 

void removeFirst(alarmNode **head) {
    // if (*head == NULL) {
    //     return; // No alarm to remove
    // }
    // alarmNode *temp = *head;
    // *head = (*head)->nextAlarm;
    
    // free(temp);
    if (*head == NULL || (*head)->nextAlarm == NULL) {
        return; // No alarm to remove or only one node remaining
    }

    alarmNode *removedNode = *head;
    *head = (*head)->nextAlarm;

    // Find the current last node
    alarmNode *lastNode = *head;
    while (lastNode->nextAlarm != NULL) {
        lastNode = lastNode->nextAlarm;
    }

    // Add the removed node to the end of the list
    removedNode->nextAlarm = NULL;
    lastNode->nextAlarm = removedNode;
}

// alarm_time_t timeDifference(alarm_time_t time1, alarm_time_t time2) {
//     int seconds1 = time1.hour * 3600 + time1.minute * 60 + time1.second;
//     int seconds2 = time2.hour * 3600 + time2.minute * 60 + time2.second;
//     int differenceInSeconds = seconds2 - seconds1;

//     alarm_time_t differenceTime;
//     differenceTime.hour = differenceInSeconds / 3600;
//     differenceTime.minute = (differenceInSeconds % 3600) / 60;
//     differenceTime.second = differenceInSeconds % 60;

//     return differenceTime;
// }
