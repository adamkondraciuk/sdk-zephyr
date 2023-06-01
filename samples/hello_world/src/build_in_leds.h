/*
 * Copyright (c) 2023 ELMODIS Ltd.
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define LED0_NODE DT_ALIAS(led1)

// TODO: Is it needed?
struct device *dev;

#ifdef HAS_BUILDIN_PWN_DIDOES
static const struct pwm_dt_spec red_pwm_l	ed =
	PWM_DT_SPEC_GET(DT_ALIAS(red_pwm_led));
static const struct pwm_dt_spec green_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(green_pwm_led));
static const struct pwm_dt_spec blue_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(blue_pwm_led));
#endif
