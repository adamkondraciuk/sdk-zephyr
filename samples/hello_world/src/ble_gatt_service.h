#ifndef BLUETOOTH_APP_H
#define BLUETOOTH_APP_H

#include <zephyr/types.h>



void bt_init(void);
void connected(struct bt_conn *conn, uint8_t err);
void disconnected(struct bt_conn *conn, uint8_t reason);
void auth_cancel(struct bt_conn *conn);
void bas_notify(void);
void hrs_notify(void);


#endif /* BLUETOOTH_APP_H */