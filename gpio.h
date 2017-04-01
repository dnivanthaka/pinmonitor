#ifndef GPIO_H
#define GPIO_H
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

#endif
