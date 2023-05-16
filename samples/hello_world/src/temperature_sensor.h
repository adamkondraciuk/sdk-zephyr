#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

const struct device *temp_dev;

void build_in_temp_init(){
    temp_dev = device_get_binding("TEMP_0");

    if (!temp_dev) {
		printf("error: no temp device\n");
		return;
	}

	printf("temp device is %p, name is %s\n",
	       temp_dev, temp_dev->name);
}

int measure_temperature(){
    int r;
    struct sensor_value temp_value;

    r = sensor_sample_fetch(temp_dev);
    if (r) {
        printf("sensor_sample_fetch failed return: %d\n", r);
    }

    r = sensor_channel_get(temp_dev, SENSOR_CHAN_AMBIENT_TEMP,
                    &temp_value);
    if (r) {
        printf("sensor_channel_get failed return: %d\n", r);
        
    }

    printf("Temperature is %gC\n",
            sensor_value_to_double(&temp_value));
    return r;
}