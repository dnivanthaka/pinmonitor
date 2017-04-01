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

#include "log.h"
#include "gpio.h"
#include "conf.h"

#define BUFF_SIZE 65
#define SYSLOG_IDEN    "PinMonitor"
#define CONF_FILE_NAME "pinmon.conf"
#define LOG_FILE_PATH  "/tmp/pinmon.log"
#define PID_FILE ""


static volatile sig_atomic_t doneflag = 1;

void show_usage()
{
    printf("pinmon -c [path to pinmon.conf]");
}

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
                show_usage();
                exit(EXIT_FAILURE);
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
        // Closing syslog and opening pinmon log 
        closelog();

        pinmon_log_open(LOG_FILE_PATH);  
        
        /* Daemon-specific initialization goes here */
        //setup_gpio(18, GPIO_IN);
        tmp = list;

        while(tmp != NULL){
            setup_gpio(tmp->gpio->pin, GPIO_IN);
            tmp->gpio->mode = 1;
            tmp->gpio->val = gpio_read(tmp->gpio->pin);

            tmp = tmp->next; 
        }

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
           
           usleep(100000); /* wait 100 mseconds */
        }
        
        //Cleanup
        tmp = list;
        while(tmp != NULL){
            cleanup_gpio(tmp->gpio->pin);
            tmp = tmp->next; 
        }
        //Closing log file
        pinmon_log_close();
        conf_free(&list);


    return 0;
}

