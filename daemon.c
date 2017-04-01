#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <string.h>

#include "gpio.h"
#include "conf.h"

#define BUFF_SIZE 65
#define SYSLOG_IDEN "PinMonitor"
#define CONF_FILE_NAME "pinmon.conf"
#define PID_FILE ""

#define GPIO_OUT 0
#define GPIO_IN  1

/*typedef struct{
    uint8_t pin;
    uint8_t mode;
    uint8_t val;
} gpio_t;*/

static volatile sig_atomic_t doneflag = 1;
static gpio_t *exported;

// Function prototypes
// GPIO
int setup_gpio(uint8_t pin, uint8_t mode);
int gpio_write(uint8_t pin, uint8_t value);
int gpio_read(uint8_t pin);
int cleanup_gpio(uint8_t pin);

// Data Logging
int opendata(int fp);
int writedata(int fp, char *data);
int closedata(int fp);

static void catchsignal(int signo, siginfo_t *siginfo, void *context)
{
    if ((signo == SIGINT) || (signo == SIGQUIT) || (signo == SIGABRT) || (signo == SIGTERM)) {
        doneflag = 0;
    }
}

int main(int argc, char *argv[])
{
        int data_fd;
        struct gpio_node *list = NULL;
        struct gpio_node *tmp = NULL;

        FILE *fp = conf_open(CONF_FILE_NAME);

        if(fp == NULL){
            if(ENOENT == errno){
                printf("No config file %s present\n", CONF_FILE_NAME);
                exit(-1);
            }    
        }
        conf_read(fp, &list);
        conf_close(fp);
        
        
        // Signal handling
        struct sigaction act;

        // Process ID and Session ID
        pid_t pid, sid;
    
        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        /* Exit the parent process. */
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);
                
        /* Opening sys log */
        openlog(SYSLOG_IDEN, LOG_PID|LOG_CONS, LOG_USER);        
                
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                /* Log the failure */
                syslog(LOG_ERR, "Cannot create a session ID");
                exit(EXIT_FAILURE);
        }
        
        
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
                /* Log the failure */
                syslog(LOG_ERR, "Cannot change the working directory");
                exit(EXIT_FAILURE);
        }
        
        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        memset (&act, 0, sizeof(act));
        act.sa_sigaction = catchsignal;/* set up signal handler */
        act.sa_flags = SA_SIGINFO;
        
        sigaction(SIGTERM, &act, NULL);
        
        /*if ((sigemptyset(&act.sa_mask) == -1) ||
            (sigaction(SIGTERM, &act, NULL) == -1)) {
            syslog(LOG_ERR, "Cannot set signal handler");
            exit(EXIT_FAILURE);
        }*/
        
        closelog();
        
        /* Daemon-specific initialization goes here */
        //setup_gpio(18, GPIO_IN);
        tmp = list;

        while(tmp != NULL){
            setup_gpio(tmp->gpio->pin, GPIO_IN);
            tmp->gpio->mode = 1;
            tmp->gpio->val = gpio_read(tmp->gpio->pin);

            tmp = tmp->next; 
        }

        
        //data_fd = open("/tmp/pinmon.txt", O_WRONLY | O_CREAT | O_TRUNC);
        
        //opendata(data_fd);
        
        //int prev_val = gpio_read(18);
        //int curr_val = gpio_read(18);
        int curr_val = 0;
        
        //char *test = "X\0";
        char buff[65];
        
        /* Daemon Loop */
        while (doneflag) {
           /* Do some task here ... */
           //{"datetime":"", "states":[{"18":"LOW"}]}
           tmp = list;

           while(tmp != NULL){
               curr_val = gpio_read(tmp->gpio->pin);
                    /*snprintf(buff, BUFF_SIZE, "Checking pin %d, prev_val = %d, new_val= %d\n", tmp->gpio->pin, tmp->gpio->val, curr_val);

                    writedata(data_fd, buff);*/
               if(curr_val != tmp->gpio->val){
                    snprintf(buff, BUFF_SIZE, "%s %d", tmp->cmd, curr_val);
                    system(buff);

                    /*snprintf(buff, BUFF_SIZE, "Script %s, len = %d\n", tmp->cmd, strlen(tmp->cmd));

                    writedata(data_fd, buff);*/
                    tmp->gpio->val = curr_val;
               }

               tmp = tmp->next; 
           }
           
           //writedata(data_fd, test);
           
           /*if(curr_val != prev_val){
                //memset(buff, 0, BUFF_SIZE);
           
                if(curr_val == 0){
                    snprintf(buff, BUFF_SIZE, "{\"datetime\":\"\", \"states\":[{\"PIN\":\"18\", \"VALUE\":\"LO\"}]}\n");
                    //writedata(data_fd, "{\"datetime\":"", \"states\":[{\"18\":\"LO\"}]}");
                }else{
                    snprintf(buff, BUFF_SIZE, "{\"datetime\":\"\", \"states\":[{\"PIN\":\"18\", \"VALUE\":\"HI\"}]}\n");
                    //writedata(data_fd, "{\"datetime\":"", \"states\":[{\"18\":\"HI\"}]}");
                }
                
                writedata(data_fd, buff);
                
                prev_val = curr_val;
           }*/
           
           
           usleep(100000); /* wait 100 mseconds */
           //sleep(1); /* wait 3 mseconds */
        }
        
        //Cleanup
        tmp = list;
        while(tmp != NULL){
            cleanup_gpio(tmp->gpio->pin);
            tmp = tmp->next; 
        }
        //closedata(data_fd);
        conf_free(&list);


    return 0;
}

// Data Logging ------------------------------------------------------//
int opendata(int fp)
{
    char *start = "-- Staring data logging --\n";
    write(fp, start, strlen(start));
}

int closedata(int fp)
{
    if(fp > 0){
        close(fp);
    }
}

int writedata(int fp, char *data)
{
    //if(fp > 0){
        write(fp, data, strlen(data));
    //}
}
// GPIO Operations ---------------------------------------------------//
int setup_gpio(uint8_t pin, uint8_t mode)
{
    int fd;
    char buff[BUFF_SIZE];
    ssize_t bytes_written;
    
    //TODO check if its already exported
    
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd < 0){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    bytes_written = snprintf(buff, BUFF_SIZE, "%d", pin);
    
    if(write(fd, buff, bytes_written) < 0){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    
    //Setting mode
    sprintf(buff, "/sys/class/gpio/gpio%d/direction", pin);
    
    fd = open(buff, O_WRONLY);
    
    if(fd < 0){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    if(mode == GPIO_OUT){
        bytes_written = snprintf(buff, BUFF_SIZE, "out");
    }else{
        bytes_written = snprintf(buff, BUFF_SIZE, "in");
    }
    
    if(write(fd, buff, bytes_written) < 0){
        // Log the error here
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
        exit(EXIT_FAILURE);
    }
    
    bytes_written = snprintf(buff, BUFF_SIZE, "%d", pin);
    
    if(write(fd, buff, bytes_written) < 0){
        // Log the error here
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
        exit(EXIT_FAILURE);
	}
 
	if (read(fd, value_str, 3) < 0) {
		// Log the error here
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
        exit(EXIT_FAILURE);
	}
	
	if(value == 1){
        snprintf(buff, 1, "1");
    }else{
        snprintf(buff, 1, "0");
    }
 
	if (write(fd, buff, 1) < 0) {
		// Log the error here
        exit(EXIT_FAILURE);
	}
 
	close(fd);
 
	return 0;
}
