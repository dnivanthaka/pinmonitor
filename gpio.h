#ifndef GPIO_H
#define GPIO_H

#define GPIO_OUT 0
#define GPIO_IN  1

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

int setup_gpio(uint8_t pin, uint8_t mode);
int cleanup_gpio(uint8_t pin);
int gpio_read(uint8_t pin);
int gpio_write(uint8_t pin, uint8_t value);
#endif
