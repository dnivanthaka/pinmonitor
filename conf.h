#ifndef CONF_H
#define CONF_H
FILE *conf_open(char *file);
int conf_read(FILE *fp, struct gpio_node **list);
int conf_close(FILE *fp);
int conf_free(struct gpio_node **list);
#endif
