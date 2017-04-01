#ifndef PINMON_LOG_H
#define PINMON_LOG_H
int pinmon_log_open(char *path);
int pinmon_log_write(char *data);
int pinmon_log_close();
#endif
