#define SECONDS_IN_YEAR_366    31622400
#define SECONDS_IN_YEAR_365    31536000
#define SECONDS_IN_MONTH_31     2678400
#define SECONDS_IN_MONTH_30     2592000
#define SECONDS_IN_FEBRUARY_29  2505600
#define SECONDS_IN_FEBRUARY_28  2419200
#define SECONDS_IN_WEEK          604800
#define SECONDS_IN_DAY            86400
#define SECONDS_IN_HOUR            3600
#define SECONDS_IN_MINUTE            60
#define SECONDS_IN_SECOND             1       // ¯\_(ツ)_/¯
#define LEAPSECOND_DELTA             -1
#define TIMEZONE_OFFSET   SECONDS_IN_HOUR * 1 // this is CET (?) or whatever

typedef struct hour_minute_second{
    int hour;
    int minute;
    int second;
}hour_minute_second_t;

hour_minute_second_t UnixTimestampToHMS(long int unix_time_stemp);
int oscilation(double frequency_hz);