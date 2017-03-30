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

#define BUFF_SIZE 65
#define SYSLOG_IDEN "PinMonitor"
#define PID_FILE ""

#define GPIO_OUT 0
#define GPIO_IN  1

typedef struct{
    uint8_t pin;
    uint8_t mode;
    uint8_t val;
} gpio_t;

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
        act.sa_sigaction = &catchsignal;/* set up signal handler */
        act.sa_flags = SA_SIGINFO;
        
        if ((sigemptyset(&act.sa_mask) == -1) ||
            (sigaction(SIGTERM, &act, NULL) == -1)) {
            syslog(LOG_ERR, "Cannot set signal handler");
            exit(EXIT_FAILURE);
        }
        
        closelog();
        
        /* Daemon-specific initialization goes here */
        setup_gpio(18, GPIO_IN);
        
        data_fd = open("/tmp/pinmon.txt", O_WRONLY | O_CREAT);
        
        opendata(data_fd);
        
        int prev_val = gpio_read(18);
        int curr_val = gpio_read(18);
        
        //char *test = "X\0";
        char buff[65];
        
        /* Daemon Loop */
        while (1) {
           /* Do some task here ... */
           //{"datetime":"", "states":[{"18":"LOW"}]}
           curr_val = gpio_read(18);
           
           //writedata(data_fd, test);
           
           if(curr_val != prev_val){
                memset(buff, 0, BUFF_SIZE);
           
                if(curr_val == 0){
                    snprintf(buff, BUFF_SIZE, "{\"datetime\":\"\", \"states\":[{\"18\":\"LO\"}]}\n");
                    //writedata(data_fd, "{\"datetime\":"", \"states\":[{\"18\":\"LO\"}]}");
                }else{
                    snprintf(buff, BUFF_SIZE, "{\"datetime\":\"\", \"states\":[{\"18\":\"HI\"}]}\n");
                    //writedata(data_fd, "{\"datetime\":"", \"states\":[{\"18\":\"HI\"}]}");
                }
                
                writedata(data_fd, buff);
                
                prev_val = curr_val;
           }
           
           
           usleep(3000); /* wait 3 mseconds */
           //sleep(1); /* wait 3 mseconds */
        }
        
        //Cleanup
        cleanup_gpio(18);
        closedata(data_fd);


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
    if(fp > 0){
        write(fp, data, strlen(data));
    }
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
