#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "log.h"
#include "gpio.h"


#define BUFF_SIZE 65

int setup_gpio(uint8_t pin, uint8_t mode)
{
    int fd;
    char buff[BUFF_SIZE];
    ssize_t bytes_written;
    
    //Check if its already open, reuse exported pin
    snprintf(buff, BUFF_SIZE, "/sys/class/gpio%d/direction", pin);
    if(open(buff, O_RDONLY) < 0){
    
        if(ENOENT == errno){
    
            fd = open("/sys/class/gpio/export", O_WRONLY);
            if(fd < 0){
                // Log the error here
                exit(EXIT_FAILURE);
            }
            
            bytes_written = snprintf(buff, BUFF_SIZE, "%d", pin);
            
            if(write(fd, buff, bytes_written) < 0){
                // Log the error here
                pinmon_log_write("Cannot export gpio pin.");
                exit(EXIT_FAILURE);
            }
            
            close(fd);
        }
    }
    
    //Setting mode
    sprintf(buff, "/sys/class/gpio/gpio%d/direction", pin);
    
    fd = open(buff, O_WRONLY);
    
    if(fd < 0){
        // Log the error here
        pinmon_log_write("Cannot open pin direction file.");
        exit(EXIT_FAILURE);
    }
    
    if(mode == GPIO_OUT){
        bytes_written = snprintf(buff, BUFF_SIZE, "out");
    }else{
        bytes_written = snprintf(buff, BUFF_SIZE, "in");
    }
    
    if(write(fd, buff, bytes_written) < 0){
        // Log the error here
        pinmon_log_write("Cannot set pin direction.");
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    
    return 0;
}

int cleanup_gpio(uint8_t pin)
{
    int fd;
    char buff[BUFF_SIZE];
    ssize_t bytes_written;
    
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd < 0){
        // Log the error here
        pinmon_log_write("Cannot open unexport file.");
        exit(EXIT_FAILURE);
    }
    
    bytes_written = snprintf(buff, BUFF_SIZE, "%d", pin);
    
    if(write(fd, buff, bytes_written) < 0){
        // Log the error here
        pinmon_log_write("Cannot write to unexport file.");
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    
    return 0;
}

int gpio_read(uint8_t pin)
{
    int fd;
    char buff[BUFF_SIZE];
    char value_str[3];

    snprintf(buff, BUFF_SIZE, "/sys/class/gpio/gpio%d/value", pin);
    
	fd = open(buff, O_RDONLY);
	if (fd < 0) {
		// Log the error here
        pinmon_log_write("Cannot open gpio value file.");
        exit(EXIT_FAILURE);
	}
 
	if (read(fd, value_str, 3) < 0) {
		// Log the error here
        pinmon_log_write("Cannot read gpio value file.");
        exit(EXIT_FAILURE);
	}
 
	close(fd);
 
	return(atoi(value_str));
}

int gpio_write(uint8_t pin, uint8_t value)
{
    int fd;
    char buff[BUFF_SIZE];
    char value_str[3];

    snprintf(buff, BUFF_SIZE, "/sys/class/gpio/gpio%d/value", pin);
    
	fd = open(buff, O_WRONLY);
	if (fd < 0) {
		// Log the error here
        pinmon_log_write("Cannot read gpio value file.");
        exit(EXIT_FAILURE);
	}
	
	if(value == 1){
        snprintf(buff, 1, "1");
    }else{
        snprintf(buff, 1, "0");
    }
 
	if (write(fd, buff, 1) < 0) {
		// Log the error here
        pinmon_log_write("Cannot write to gpio value file.");
        exit(EXIT_FAILURE);
	}
 
	close(fd);
 
	return 0;
}
