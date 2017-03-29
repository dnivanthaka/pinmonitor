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

static volatile sig_atomic_t doneflag = 0;
static gpio_t *exported;


static void catchsignal(int sig, siginfo_t *siginfo, void *context)
{
    doneflag = 1;
}

// Function prototypes
// GPIO
int setup_gpio(uint8_t pin, uint8_t mode);
int gpio_write();
int gpio_read();
int cleanup_gpio();

// Data Logging
int opendata();
int writedata();
int closedata();

int main(int argc, char *argv[])
{
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
        
        memset (&act, '\0', sizeof(act));
        act.sa_sigaction = &catchsignal;/* set up signal handler */
        act.sa_flags = SA_SIGINFO;
        
        if ((sigemptyset(&act.sa_mask) == -1) ||
            (sigaction(SIGINT, &act, NULL) == -1)) {
            syslog(LOG_ERR, "Cannot set signal handler");
            exit(EXIT_FAILURE);
        }
        
        closelog();
        
        /* Daemon-specific initialization goes here */
        setup_gpio(18, GPIO_IN);
        
        /* Daemon Loop */
        while (1) {
           /* Do some task here ... */
           
           usleep(3000); /* wait 3 mseconds */
        }


    return 0;
}

int setup_gpio(uint8_t pin, uint8_t mode)
{
    int fd;
    char buff[BUFF_SIZE];
    ssize_t bytes_written;
    
    //TODO check if its already exported
    
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fp < 0){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    bytes_written = snprintf(buff, BUFF_SIZE, "%d", pin);
    
    if(write(fd, buff, bytes_written) < bytes_written){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    close(fp);
    
    //Setting mode
    sprintf(buff, "/sys/class/gpio/gpio%d/direction", pin);
    
    fd = open(buff, O_WRONLY);
    
    if(fp < 0){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    if(mode == GPIO_OUT){
        bytes_written = snprintf(buff, BUFF_SIZE, "out");
    }else{
        bytes_written = snprintf(buff, BUFF_SIZE, "in");
    }
    
    if(write(fd, buff, bytes_written) < bytes_written){
        // Log the error here
        exit(EXIT_FAILURE);
    }
    
    close(fp);
}
