#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

static int logfd;

int pinmon_log_open(char *path)
{
    logfd = open(path, O_WRONLY | O_CREAT | O_APPEND);
    char *start = "-- PINMON STARTING --\n";
    write(logfd, start, strlen(start));
}

int pinmon_log_close()
{
    if(logfd > 0){
        close(logfd);
    }
}

int pinmon_log_write(char *data)
{
    //if(fp > 0){
        write(logfd, data, strlen(data));
    //}
}
