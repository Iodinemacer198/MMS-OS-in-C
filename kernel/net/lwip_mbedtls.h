#ifndef LWIP_MBEDTLS_H
#define LWIP_MBEDTLS_H

#include <stdbool.h>
#include <stdint.h>

#define NET_MAX_HOST 128
#define NET_MAX_PATH 160
#define NET_MAX_BODY 512
#define NET_MAX_TOKEN 192
#define NET_MAX_CHANNEL 64

typedef struct {
    bool initialized;
    bool qemu_bridge_ready;
    char token[NET_MAX_TOKEN];
    char channel_id[NET_MAX_CHANNEL];
} NetRuntime;

bool net_stack_init(void);
NetRuntime* net_runtime_get(void);

bool net_set_bot_token(const char* token);
bool net_set_channel(const char* channel);

bool net_discord_post_message(const char* message, int* status_code);
bool net_discord_gateway_ping(int* status_code);

#endif
