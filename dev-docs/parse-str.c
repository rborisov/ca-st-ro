#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum message_type
{
    gps_speed,
    notification_new_file,
    notification_deleted_file,
    notification_mnt_point,
    unknown
};

enum message_type get_type_message(const char *buffer, char **message)
{
    enum message_type type = unknown;
    char *start = buffer;
    char *end = buffer;
    *message = start;
    while (*buffer) {
        if (*buffer == '[') start = buffer;
        else if (*buffer == ']') end = buffer;
        if (start < end && *start) {
            *end = 0;
            printf("start: %s\n", start+1);
            printf("end: %s\n", end+1);
            if (strcmp(start+1, "newfile") == 0) type = notification_new_file;
            else if (strcmp(start+1, "mntpoint") == 0) type = notification_mnt_point;
            else if (strcmp(start+1, "speed") == 0) type = gps_speed;
            *message = end+1;
            printf("message: %s\n", *message);
            *end = ']';
            break;
        }
        buffer++;
    }
    return type;
}

void main(void)
{
    char buffer[128] = "[newfile]buffer";
    char buffer1[128] = "[mntpoint]buffer1";
    char buffer2[128] = "[speed]buffer2";
    char buffer3[128] = "buffer3";
    int type;
    char *message;

    type = get_type_message(buffer, &message);
    printf("%d %s %s\n", type, message, buffer);
    type = get_type_message(buffer1, &message);
    printf("%d %s %s\n", type, message, buffer1);
    type = get_type_message(buffer2, &message);
    printf("%d %s %s\n", type, message, buffer2);
    type = get_type_message(buffer3, &message);
    printf("%d %s %s\n", type, message, buffer3);
}
