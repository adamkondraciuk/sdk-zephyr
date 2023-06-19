#ifndef ADC_H
#define ADC_H

typedef struct {
    bool (* initialize)(void * );
    bool (* configure)(void *);
    bool (* start) (void *);
    #warning add others
} adc_t;

#define ADC_INITIALIZE_STRUCT(               \
            init_func,                       \
            config_func,                     \
            start_func,                      \
            stop_func,                       \
            read_func)                       \
        {                                    \
            .initialize = init_func,         \
            .configure = config_func,        \
            .start = start_func,             \
        }

#endif // ADC_H
