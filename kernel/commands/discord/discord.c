#include "discord.h"
#include "lwip_mbedtls.h"

extern void print(const char* str);
extern void println(const char* str);
extern void printint(int num);

static int str_len(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

static bool starts_with(const char* s, const char* prefix) {
    int i = 0;
    while (prefix[i] != '\0') {
        if (s[i] != prefix[i]) return false;
        i++;
    }
    return true;
}

static const char* skip_spaces(const char* s) {
    while (*s == ' ') s++;
    return s;
}

void discord_print_status(void) {
    NetRuntime* rt = net_runtime_get();

    println("Discord stack status:");
    print(" - Stack initialized: ");
    println(rt->initialized ? "yes" : "no");

    print(" - QEMU bridge ready: ");
    println(rt->qemu_bridge_ready ? "yes" : "no");

    print(" - Bot token loaded: ");
    println(rt->token[0] != '\0' ? "yes" : "no");

    print(" - Channel set: ");
    if (rt->channel_id[0] != '\0') {
        println(rt->channel_id);
    } else {
        println("no");
    }
}

void discord_command(const char* input) {
    const char* cmd = skip_spaces(input);

    if (cmd[0] == '\0' || starts_with(cmd, "help")) {
        println("discord help");
        println("discord status");
        println("discord token <BOT_TOKEN>");
        println("discord channel <CHANNEL_ID>");
        println("discord ping");
        println("discord send <message>");
        return;
    }

    if (starts_with(cmd, "status")) {
        discord_print_status();
        return;
    }

    if (starts_with(cmd, "token ")) {
        const char* token = skip_spaces(cmd + 6);
        if (str_len(token) == 0) {
            println("Token cannot be empty.");
            return;
        }

        if (net_set_bot_token(token)) {
            println("Bot token set.");
        } else {
            println("Failed to set token. Run 'netinit' first.");
        }
        return;
    }

    if (starts_with(cmd, "channel ")) {
        const char* channel = skip_spaces(cmd + 8);
        if (str_len(channel) == 0) {
            println("Channel ID cannot be empty.");
            return;
        }

        if (net_set_channel(channel)) {
            println("Channel ID set.");
        } else {
            println("Failed to set channel. Run 'netinit' first.");
        }
        return;
    }

    if (starts_with(cmd, "ping")) {
        int code = 0;
        if (net_discord_gateway_ping(&code)) {
            println("Gateway request emitted to QEMU bridge.");
            if (code > 0) {
                print("Upstream HTTP status: ");
                printint(code);
                println("");
            } else {
                println("No upstream status available in-kernel.");
            }
        } else {
            println("Ping failed. Ensure netinit and token are configured.");
        }
        return;
    }

    if (starts_with(cmd, "send ")) {
        const char* message = skip_spaces(cmd + 5);
        if (str_len(message) == 0) {
            println("Message cannot be empty.");
            return;
        }

        int code = 0;
        if (net_discord_post_message(message, &code)) {
            println("Discord message emitted to QEMU bridge.");
            if (code > 0) {
                print("Upstream HTTP status: ");
                printint(code);
                println("");
            } else {
                println("No upstream status available in-kernel.");
            }
        } else {
            println("Send failed. Ensure netinit, token, and channel are configured.");
        }
        return;
    }

    println("Unknown discord command. Use 'discord help'.");
}
