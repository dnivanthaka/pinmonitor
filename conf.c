#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "gpio.h"
#include "conf.h"


FILE *conf_open(char *file)
{
    FILE *fp;

    fp = fopen(file, "r");
    return fp;
}

int conf_read(FILE *fp, struct gpio_node **list)
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
                //buff[len2 - 1] = '\0';
                //len2 = len2 - 1;

                printf("%s\n", buff);
                printf("%s\n", &buff[len1+1]);
                printf("Len = %d\n", strlen(&buff[len1+1]));
                gpio_t *gpdata = malloc(sizeof(gpio_t));
                gpdata->pin = atoi(buff);
                gpdata->mode = 0;
                gpdata->val  = 0;

                //struct gpio_node *tmp;
                //tmp = *list;

                if((*list) != NULL){
                    list = &((*list)->next);
                    
                }

                (*list) = (struct gpio_node *)malloc(sizeof(struct gpio_node));
                //strlen returns the length of the string without the null terminator
                char *str = malloc(len2 + 1);

                memcpy(str, &buff[len1 + 1], len2 + 1);

                (*list)->gpio = gpdata;
                (*list)->cmd  = str;
                (*list)->next = NULL;
                printf("Copy = %s\n", str);

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

int conf_free(struct gpio_node **list)
{
    struct gpio_node *tmp;

    while((*list) != NULL){
        tmp = *list;
        list = &((*list)->next);
        free(tmp);
    }
}

/*void test_mem(struct gpio_node **list){
    
    (*list) = (struct gpio_node *)malloc(sizeof(struct gpio_node));
    (*list)->next = NULL;
}*/
//Added for testing purposes
/*int main(void){
    struct gpio_node *list = NULL;

    FILE *fp = conf_open("test.conf");

    if(fp == NULL){
        if(ENOENT == errno){
            printf("No config file present\n");
            exit(EXIT_SUCCESS);
        }    
    }
    conf_read(fp, &list);
    conf_close(fp);

    //test_mem(&list);

    struct gpio_node *tmp;

    //tmp = list;

    //printf("%d\n", list);

    while(list != NULL){
        printf("Pin = %d\n", list->gpio->pin);
        list = list->next; 
    }

    conf_free(&list);

    return 0;
}*/
