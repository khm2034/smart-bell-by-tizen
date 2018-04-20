#include <sys/time.h>
#include <time.h>
#include "log.h"
#include "resource/resource_PCA9685.h"
#include "resource/resource_passive_buzzer.h"
#include "smartbell/smartbell_util.h"

int buzzer_state = 0;
static int resource_passive_buzzer_init(unsigned int ch)
{
	int ret = 0;
	ret = resource_pca9685_init(ch);
	if (ret) {
		_E("failed to init PCA9685 with ch[%u]", ch);
		return -1;
	}
	buzzer_state = 1;
	return 0;
}

void resource_close_passive_buzzer(unsigned int ch)
{
	if (buzzer_state == 1) {
		resource_pca9685_fini(ch);
		buzzer_state = 0;
	}

	return;
}

int resource_set_passive_buzzer_value(unsigned int ch, int value, float beat)
{
	int ret = 0;

	if (buzzer_state == 0) {
		ret = resource_passive_buzzer_init(ch);
		if (ret)
			return -1;
		ret = resource_pca9685_set_value_to_channel(ch, 0, 2048);
		if (ret)
			return -1;
	}
	ret = resource_pca9685_set_frequency(value);
	if(ret == -1)
		return -1;
	if(value == 0 && beat == 0)
		resource_close_passive_buzzer(ch);
	else
		delay(2000*beat);

	return ret;
}
