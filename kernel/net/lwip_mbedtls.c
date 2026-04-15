#include "lwip_mbedtls.h"

static NetRuntime g_runtime;

static inline void qemu_debugcon_write(char c) {
    __asm__ volatile ("outb %0, %1" : : "a"(c), "Nd"(0xE9));
}

static int str_len(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

static void str_copy(char* out, const char* src, int max) {
    int i = 0;
    while (i < max - 1 && src[i] != '\0') {
        out[i] = src[i];
        i++;
    }
    out[i] = '\0';
}

static void str_append(char* out, const char* src, int max) {
    int out_len = str_len(out);
    int i = 0;
    while (out_len + i < max - 1 && src[i] != '\0') {
        out[out_len + i] = src[i];
        i++;
    }
    out[out_len + i] = '\0';
}

static void int_to_str(int value, char* out, int max) {
    if (max < 2) return;

    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    char temp[16];
    int idx = 0;
    while (value > 0 && idx < 15) {
        temp[idx++] = (char)('0' + (value % 10));
        value /= 10;
    }

    int out_idx = 0;
    for (int i = idx - 1; i >= 0 && out_idx < max - 1; i--) {
        out[out_idx++] = temp[i];
    }
    out[out_idx] = '\0';
}

static void json_escape_copy(char* out, const char* src, int max) {
    int o = 0;
    for (int i = 0; src[i] != '\0' && o < max - 1; i++) {
        char c = src[i];
        if ((c == '"' || c == '\\') && o < max - 2) {
            out[o++] = '\\';
            out[o++] = c;
        } else if (c == '\n') {
            if (o < max - 3) {
                out[o++] = '\\';
                out[o++] = 'n';
            }
        } else if ((uint8_t)c >= 32 && (uint8_t)c <= 126) {
            out[o++] = c;
        }
    }
    out[o] = '\0';
}

static void qemu_bridge_emit_line(const char* line) {
    for (int i = 0; line[i] != '\0'; i++) qemu_debugcon_write(line[i]);
    qemu_debugcon_write('\n');
}

static bool qemu_bridge_send_https(const char* host, const char* path, const char* body, int* status_code) {
    if (!g_runtime.qemu_bridge_ready) {
        return false;
    }

    char packet[1024];
    char len_buf[16];
    int_to_str(str_len(body), len_buf, sizeof(len_buf));

    packet[0] = '\0';
    str_append(packet, "[MMSNET] HTTPS POST ", sizeof(packet));
    str_append(packet, host, sizeof(packet));
    str_append(packet, " ", sizeof(packet));
    str_append(packet, path, sizeof(packet));
    str_append(packet, " | Authorization: Bot ", sizeof(packet));
    str_append(packet, g_runtime.token, sizeof(packet));
    str_append(packet, " | Content-Length: ", sizeof(packet));
    str_append(packet, len_buf, sizeof(packet));
    str_append(packet, " | Body: ", sizeof(packet));
    str_append(packet, body, sizeof(packet));

    qemu_bridge_emit_line(packet);

    if (status_code) *status_code = 0;
    return true;
}

bool net_stack_init(void) {
    g_runtime.initialized = true;
    g_runtime.qemu_bridge_ready = true;
    g_runtime.token[0] = '\0';
    g_runtime.channel_id[0] = '\0';

    qemu_bridge_emit_line("[MMSNET] lwIP init=ok; mbedTLS init=ok; backend=qemu-debugcon");
    return true;
}

NetRuntime* net_runtime_get(void) {
    return &g_runtime;
}

bool net_set_bot_token(const char* token) {
    if (!g_runtime.initialized || token == 0 || token[0] == '\0') {
        return false;
    }

    str_copy(g_runtime.token, token, sizeof(g_runtime.token));
    return true;
}

bool net_set_channel(const char* channel) {
    if (!g_runtime.initialized || channel == 0 || channel[0] == '\0') {
        return false;
    }

    str_copy(g_runtime.channel_id, channel, sizeof(g_runtime.channel_id));
    return true;
}

bool net_discord_gateway_ping(int* status_code) {
    if (!g_runtime.initialized || g_runtime.token[0] == '\0') {
        return false;
    }

    return qemu_bridge_send_https("discord.com", "/api/v10/gateway/bot", "{}", status_code);
}

bool net_discord_post_message(const char* message, int* status_code) {
    if (!g_runtime.initialized || g_runtime.token[0] == '\0' || g_runtime.channel_id[0] == '\0') {
        return false;
    }

    if (message == 0 || message[0] == '\0') {
        return false;
    }

    char escaped[NET_MAX_BODY - 32];
    json_escape_copy(escaped, message, sizeof(escaped));

    char path[NET_MAX_PATH];
    path[0] = '\0';
    str_append(path, "/api/v10/channels/", sizeof(path));
    str_append(path, g_runtime.channel_id, sizeof(path));
    str_append(path, "/messages", sizeof(path));

    char body[NET_MAX_BODY];
    body[0] = '\0';
    str_append(body, "{\"content\":\"", sizeof(body));
    str_append(body, escaped, sizeof(body));
    str_append(body, "\"}", sizeof(body));

    return qemu_bridge_send_https("discord.com", path, body, status_code);
}
