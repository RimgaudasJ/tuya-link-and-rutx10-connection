
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

static struct ubus_context *local_ctx;

int ubus_init_connection(struct ubus_context **ctx)
{
    *ctx = ubus_connect(NULL);
    if (!*ctx) {
        syslog(LOG_ERR, "Failed to connect to ubus");
        return -1;
    }
    syslog(LOG_INFO, "Connected to ubus");
    local_ctx = *ctx;
    return 0;
}

/* Callback to handle reply */
static void ubus_call_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    (void)type; // Unused parameter
    struct blob_attr **resp = (struct blob_attr **)req->priv;

    if (!msg || !resp)
        return;

    /* Duplicate message so it remains valid after callback */
    *resp = blob_memdup(msg);

    char *str = blobmsg_format_json(msg, true);
    syslog(LOG_INFO, "Received reply:\n%s\n", str);
    free(str);
}

/* Call a ubus method */
int ubus_call_method(const char *method,struct blob_attr *params,struct blob_attr **response)
{
    uint32_t id;
    int ret;
    const char *path = "esp_module";
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    int rem;
    struct blob_attr *attr;
    blobmsg_for_each_attr(attr, params, rem) {
        syslog(LOG_DEBUG, "Adding param: %s", blobmsg_name(attr));
        blobmsg_add_blob(&b, attr);
    }


    /* Lookup object ID */
    ret = ubus_lookup_id(local_ctx, path, &id);
    if (ret != UBUS_STATUS_OK) {
        syslog(LOG_ERR, "Failed to lookup '%s': %s", path, ubus_strerror(ret));
        return ret;
    }

    syslog(LOG_INFO, "Calling %s %s %s", path, method, params ? blobmsg_format_json(params, true) : "{}");

    /* Call the method */
    ret = ubus_invoke(local_ctx, id, method, b.head, ubus_call_cb, response, 3000);
    if (ret != UBUS_STATUS_OK) {
        syslog(LOG_ERR, "Invoke failed: %s", ubus_strerror(ret));
    }

    blob_buf_free(&b);
    return ret;
}

