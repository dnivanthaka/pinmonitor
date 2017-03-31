#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "conf.h"

typedef struct{
    uint8_t pin;
    uint8_t mode;
    uint8_t val;
} gpio_t;

struct gpio_node{
    gpio_t *gpio;
    char *cmd;
    struct gpio_node *next;
};


struct gpio_node *list;

FILE *conf_open(char *file)
{
    FILE *fp;

    fp = fopen(file, "r");

    list = NULL;

    return fp;
}

int conf_read(FILE *fp)
{
    char buff[75];
    int bytes_read = 0;
    char pinDefs = 0;
    char *ptr;

    while(fgets(buff, sizeof(buff), fp) != NULL){
        //Removing newline
        buff[strlen(buff) - 1] = '\0';
        //printf("Line => %s\n", buff);
        //Looking for pin mappings
        if(strcmp("[Pins]", buff) == 0){
             pinDefs = 1;
        } 

        if(pinDefs == 1){
            if((ptr = strchr(buff, ',')) != NULL){
                int len1, len2;

                *ptr = '\0';

                len1 = strlen(buff); 
                len2 = strlen(&buff[len1 + 1]);

                printf("%s\n", buff);
                //printf("%s\n", &buff[len1+1]);
                gpio_t *gpdata = malloc(sizeof(gpio_t));
                gpdata->pin = atoi(buff);
                gpdata->mode = 0;
                gpdata->val  = 0;


                    if(list == NULL){
                        list = (struct gpio_node *)malloc(sizeof(struct gpio_node));
                        char *str = malloc(strlen(&buff[len1 + 1]));

                        memcpy(str, &buff[len1 + 1], len2);

                        list->gpio = gpdata;
                        list->cmd  = str;
                        list->next = NULL;
                    }else{
                        struct gpio_node *tmp;
                        tmp = list;
                        //Goto last node
                        while(tmp->next != NULL){
                            tmp = tmp->next; 
                        }

                        tmp->next = (struct gpio_node *)malloc(sizeof(struct gpio_node));
                        tmp = tmp->next;
                        char *str = malloc(strlen(&buff[len1 + 1]));

                        memcpy(str, &buff[len1 + 1], len2);

                        tmp->gpio = gpdata;
                        tmp->cmd  = str;
                        tmp->next = NULL;
                    }



            } 
        }
    }

}

int conf_close(FILE *fp)
{
    //if(fd > 0){
        fclose(fp);
    //}
}

int main(void){
    FILE *fp = conf_open("test.conf");
    conf_read(fp);
    conf_close(fp);

    struct gpio_node *tmp;

    tmp = list;

    //printf("%d\n", list);

    while(tmp != NULL){
        printf("Pin = %d\n", tmp->gpio->pin);
        tmp = tmp->next; 
    }

    return 0;
}
