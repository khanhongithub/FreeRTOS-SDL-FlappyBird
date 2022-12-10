/**
 * @file timef.c
 * @author Nino-tum
 * @date November 2022
 * @brief  provides a function for converting UNIX time 
 * into a more readable format
 */
#include "timef.h"
#include "FreeRTOSConfig.h" // required to get tps

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

hour_minute_second_t UnixTimestampToHMS(long int unix_time_stemp)
{
    int second_in_day, hour, minute, second;
    /* 
       since every day since 1.1.1970 (theoretically) 
       took the same amount of seconds, all those days
       can be stripped away with modulo SECONDS_IN_DAY
    */
    second_in_day = unix_time_stemp % SECONDS_IN_DAY;
    second = second_in_day % SECONDS_IN_MINUTE;
    minute = ((second_in_day % SECONDS_IN_HOUR) - second) 
              / SECONDS_IN_MINUTE;
    hour = (second_in_day - minute * SECONDS_IN_MINUTE - second)
            / SECONDS_IN_HOUR;
            
    // time is returned in a custom format 
    return (hour_minute_second_t) { hour, minute, second };
}

int oscilation(double frequency_hz)
{
    int duration_in_ticks = pdMS_TO_TICKS(1 / frequency_hz * 1000);
    // int duration_in_ticks = (int)((1 / frequency_hz) * configTICK_RATE_HZ);
    int half_duration_in_ticks = duration_in_ticks >> 1;
    return 
    (((int)xTaskGetTickCount() % duration_in_ticks) <= half_duration_in_ticks);
}