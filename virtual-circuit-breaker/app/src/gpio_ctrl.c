#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include "gpio_ctrl.h"

/**
 * @brief exports the GPIO PIN to be used as input or output
 * 
 * @param pin_number string with the pin number to be exported
 * @return int 0 if the pin is exported correctly or -1 if there is an error
 */
int export_pin(uint8_t pin_number)
{
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        syslog(LOG_ERR, "Unable to open /sys/class/gpio/export");
        return -1;
    }

    char gpio_pin_str [6];

    sprintf(gpio_pin_str, "%d", pin_number);

    if (write(fd, gpio_pin_str, 2) != 2) {
        syslog(LOG_ERR, "Error writing to /sys/class/gpio/export");
        return -1;
    }

    close(fd);

    return 0;
}

/**
 * @brief Set the pin direction object
 * 
 * @param pin_number pin_number
 * @param direction desired pin direction. Posible options are "out" or "in"
 * @return int 0 if the pin direction is set correctly or -1 if there is an error
 */
int set_pin_direction(uint8_t pin_number, char* direction)
{
    char gpio_dir_path [35];

    sprintf(gpio_dir_path, "/sys/class/gpio/gpio%d/direction", pin_number);

    int fd = open(gpio_dir_path, O_WRONLY);

    if (fd == -1) {
        syslog(LOG_ERR,"Unable to open %s", gpio_dir_path);
        return -1;
    }

    if (write(fd, direction, 3) != 3) {
        syslog(LOG_ERR,"Error writing to %s", gpio_dir_path);
        return -1;
    }

    close(fd);

    return 0;
}

/**
 * @brief Set pin value
 * 
 * @param pin_number pin_number
 * @param value desired value por the pin. true (on) or false (off)
 * @return int 0 if the pin value is writen correctly or -1 if there is an error
 */
int write_pin_value (uint8_t pin_number, bool value)
{
    char gpio_dir_path [35];

    sprintf(gpio_dir_path, "/sys/class/gpio/gpio%d/value", pin_number);

    int fd = open(gpio_dir_path, O_WRONLY);

    if (fd == -1) {
        syslog(LOG_ERR,"Unable to open %s", gpio_dir_path);
        return -1;
    }

    if (value == true)
    {
        if (write(fd, "1", 1) != 1) 
        {
            syslog(LOG_ERR, "Error writing to %s", gpio_dir_path);
            return -1;
        }
    }
    else
    {
        if (write(fd, "0", 1) != 1) 
        {
            syslog(LOG_ERR, "Error writing to %s", gpio_dir_path);
            return -1;
        }
    }

    close(fd);

    return 0;
}