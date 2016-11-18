#include <stdio.h>
#include <time.h>

void main()
{
    char timestamp[] = "2015-1333-31 12:42:25";
    struct tm tm;
    if ( strptime(timestamp, "%Y-%m-%d %H:%M:%S", &tm) != NULL ) {
        printf("%d-%d-%d %d:%d:%d\n", tm.tm_year, tm.tm_mon, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

}
