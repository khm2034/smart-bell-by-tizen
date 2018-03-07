
#include <stdlib.h>
#include <unistd.h>
#include <peripheral_io.h>

#include "log.h"
#include "resource_internal.h"

void resource_close_buzzer(int pin_num)
{
	if (!resource_get_info(pin_num)->opened) return;

	_I("BUZZER is finishing...");
	peripheral_gpio_close(resource_get_info(pin_num)->sensor_h);
	resource_get_info(pin_num)->opened = 0;
}

int resource_write_buzzer(int pin_num, int write_value)
{
	int ret = PERIPHERAL_ERROR_NONE;

	if (!resource_get_info(pin_num)->opened) {
		ret = peripheral_gpio_open(pin_num, &resource_get_info(pin_num)->sensor_h);
		retv_if(!resource_get_info(pin_num)->sensor_h, -1);

		ret = peripheral_gpio_set_direction(resource_get_info(pin_num)->sensor_h, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
		retv_if(ret != 0, -1);

		resource_get_info(pin_num)->opened = 1;
		resource_get_info(pin_num)->close = resource_close_buzzer;
	}

	ret = peripheral_gpio_write(resource_get_info(pin_num)->sensor_h, write_value);
	retv_if(ret < 0, -1);

	//_I("BUZZER Value : %s", write_value ? "ON":"OFF");

	return 0;
}
