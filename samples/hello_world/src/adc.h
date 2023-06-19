#ifndef ADC_H
#define ADC_H

typedef struct {
    bool (* initialize)(void * );
    bool (* configure)(void *);
    bool (* start) (void *);
} adc_t;

#define ADC_INITIALIZE_STRUCT(               \
            init_func,                       \
            config_func,                     \
            start_func)                      \
        {                                    \
            .initialize = init_func,         \
            .configure = config_func,        \
            .start = start_func,             \
        }

#endif // ADC_H