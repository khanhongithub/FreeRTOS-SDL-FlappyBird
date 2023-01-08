#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

#define host true
#define client false

typedef bool connection_mode;

typedef char ipv4[16];

typedef struct multiplayer_config {
    connection_mode type; // <- either host or client

    ipv4 own_ip[16]; // <- own instance
    ipv4 connection_endpoint_ip[16]; // <- instance that is being connected to

}multiplayer_config_t;

void MultiplayerConfigEnter(void);
void MultiplayerConfigRun(void);
void MultiplayerConfigExit(void);
void vMultiplayerConfigTask(void *pvParameters);