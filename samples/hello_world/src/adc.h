#ifndef ADC_H
#define ADC_H

typedef struct {
    bool (*initialize)(void *);
    bool (*configure)(void *);
    bool (*start) (void *);
    bool (*stop) (void *);
    bool (*channel_read) (uint8_t chan, void *);
    bool (*all_channels_read) (void *);
#if IS_ENABLED(CONFIG_ADC_CHANNEL_NAMES_USE)
    const char *channel_name[CONFIG_ADC_MAX_NUMBER_OF_CHANNELS];
#endif
} adc_t;

#define ADC_INITIALIZE_STRUCT(        			 \
            init_func,                			 \
            config_func,              			 \
            start_func,               			 \
            stop_func,                			 \
            channel_read_func,				 \
	    all_channels_read_func)                	 \
        {                             			 \
            .initialize = init_func,  			 \
            .configure = config_func, 			 \
            .start = start_func,      			 \
	    .stop = stop_func,        			 \
	    .channel_read = channel_read_func, 		 \
	    .all_channels_read = all_channels_read_func  \
        }

#if IS_ENABLED(CONFIG_ADC_CHANNEL_NAMES_USE)
#define ADC_CHANNEL_NAME_ASSIGN(adc, channel, name) \
	adc.channel_name[channel] = name

#define ADC_CHANNEL_NAME_GET(adc, channel) \
	adc.channel_name[channel]
#endif

#endif // ADC_H
