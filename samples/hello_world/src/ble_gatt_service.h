#ifndef BLUETOOTH_APP_H
#define BLUETOOTH_APP_H

#include <zephyr/types.h>



void bt_init(void);

void bas_notify(void);
void hrs_notify(void);


#endif /* BLUETOOTH_APP_H */