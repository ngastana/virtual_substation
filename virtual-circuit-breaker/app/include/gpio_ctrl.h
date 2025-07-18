#ifndef GPIO_CTRL_H_
#define GPIO_CTRL_H_
#include <stdbool.h>
#include <stdint.h>

int export_pin(uint8_t pin_number);

int set_pin_direction(uint8_t pin_number, char* direction);

int write_pin_value (uint8_t pin_number, bool value);

#endif
