#include <signal.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <string.h>
#include <syslog.h>
#include "hardware.h"

enum {
    SYS_INFO_MEMORY,
    SYS_INFO_UPTIME,
    __SYS_INFO_MAX
};

enum {
    SYS_MEM_TOTAL,
    SYS_MEM_FREE,
    __SYS_MEM_MAX
};

static const struct blobmsg_policy system_info_policy[__SYS_INFO_MAX] = {
    [SYS_INFO_MEMORY] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
    [SYS_INFO_UPTIME] = { .name = "uptime", .type = BLOBMSG_TYPE_INT32 },
};

static const struct blobmsg_policy memory_policy[__SYS_MEM_MAX] = {
    [SYS_MEM_TOTAL] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
    [SYS_MEM_FREE] = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
};

static void system_info_callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
    system_metrics_t *out;
    struct blob_attr *tb[__SYS_INFO_MAX] = {0};
    struct blob_attr *mem[__SYS_MEM_MAX] = {0};
    (void)type;

    if (!req || !req->priv || !msg) {
        syslog(LOG_ERR, "Invalid callback args");
        return;
    }

    out = (system_metrics_t *)req->priv;

    if (blobmsg_parse(system_info_policy, __SYS_INFO_MAX, tb, blob_data(msg), blob_len(msg)) != 0) {
        syslog(LOG_ERR, "blobmsg_parse failed");
        return;
    }
    
    if (tb[SYS_INFO_UPTIME])
        out->uptime_secs = blobmsg_get_u32(tb[SYS_INFO_UPTIME]);

    if (tb[SYS_INFO_MEMORY]) {
        if (blobmsg_parse(memory_policy, __SYS_MEM_MAX, mem,
                          blobmsg_data(tb[SYS_INFO_MEMORY]),
                          blobmsg_data_len(tb[SYS_INFO_MEMORY])) == 0) {
            if (mem[SYS_MEM_TOTAL])
                out->total_ram = blobmsg_get_u64(mem[SYS_MEM_TOTAL]);
            if (mem[SYS_MEM_FREE])
                out->free_ram = blobmsg_get_u64(mem[SYS_MEM_FREE]);
        } else {
            syslog(LOG_ERR, "blobmsg_parse memory failed");
        }
    }
}

int get_system_metrics(system_metrics_t *out)
{
    if (!out)
        return -1;

    memset(out, 0, sizeof(*out));
    uint32_t id;
    int ret;

    struct ubus_context *ctx = ubus_connect(NULL);
    if (!ctx) {
        syslog(LOG_ERR, "Failed to connect to ubus");
        return -1;
    }

    ret = ubus_lookup_id(ctx, "system", &id);
    if (ret) {
        syslog(LOG_ERR, "Failed to lookup system object");
        ubus_free(ctx);
        return -1;
    }
    ret = ubus_invoke(ctx, id, "info", NULL,
        system_info_callback,
        out,
        3000
    );
    ubus_free(ctx);
    return ret;
}