#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/usb_handler.h"
#include "include/ubus_handler.h"
#include <string.h>


const struct blobmsg_policy on_policy[] = {
    [MSG_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32},
    [MSG_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING}
};
const struct blobmsg_policy off_policy[] = {
    [MSG_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32},
    [MSG_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING}
};
const struct blobmsg_policy get_policy[] = {
    [MSG_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32},
    [MSG_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING},
    [MSG_MODEL] = { .name = "model", .type = BLOBMSG_TYPE_STRING},
    [MSG_SENSOR] = { .name = "sensor", .type = BLOBMSG_TYPE_STRING}
};

static int send_serial_request(struct ubus_context *ctx,
                               struct ubus_request_data *req,
                               const char *port,
                               const char *json)
{
    char *reply_json = NULL;
    struct blob_buf reply_buf;
    if (!ctx || !req || !port || port[0] == '\0' || !json) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    if (send_and_get_reply(port, json, &reply_json) != UBUS_STATUS_OK) {
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    if (!reply_json || reply_json[0] == '\0') {
        reply_json = strdup("{}");
    }

    memset(&reply_buf, 0, sizeof(reply_buf));
    blob_buf_init(&reply_buf, 0);
    if (!blobmsg_add_json_from_string(&reply_buf, reply_json)) {
        blob_buf_free(&reply_buf);
        free(reply_json);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    ubus_send_reply(ctx, req, reply_buf.head);
    blob_buf_free(&reply_buf);
    free(reply_json);

    return UBUS_STATUS_OK;
}

int on_method(struct ubus_context *ctx, struct ubus_object *obj,
                     struct ubus_request_data *req, const char *method,
                     struct blob_attr *msg)
{
    (void)ctx; (void)obj; (void)req; (void)method;
    struct blob_attr *tb[__MSG_MAX] = {0};

    if (!msg) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    blobmsg_parse(on_policy, ARRAY_SIZE(on_policy),
              tb,
              blob_data(msg),
              blob_len(msg));

    if (!tb[MSG_PIN] || !tb[MSG_PORT]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    int pin = blobmsg_get_u32(tb[MSG_PIN]);
    const char* port = blobmsg_get_string(tb[MSG_PORT]);
    if (!port || port[0] == '\0') {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    struct blob_buf bbuf;
    memset(&bbuf, 0, sizeof(bbuf));
    blob_buf_init(&bbuf, 0);
    blobmsg_add_string(&bbuf, "action", "on");
    blobmsg_add_u32(&bbuf, "pin", pin);
    blobmsg_add_string(&bbuf, "port", port);

    char* json = blobmsg_format_json(bbuf.head, true);
    int status;

    if (!json) {
        blob_buf_free(&bbuf);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    status = send_serial_request(ctx, req, port, json);

    blob_buf_free(&bbuf);
    free((void*)json);

    return status;
}

int off_method(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg)
{
    (void)ctx; (void)obj; (void)req; (void)method;
    struct blob_attr *tb[__MSG_MAX] = {0};

    if (!msg) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    blobmsg_parse(off_policy, ARRAY_SIZE(off_policy),
              tb,
              blob_data(msg),
              blob_len(msg));

    if (!tb[MSG_PIN] || !tb[MSG_PORT]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    int pin = blobmsg_get_u32(tb[MSG_PIN]);
    const char* port = blobmsg_get_string(tb[MSG_PORT]);
    if (!port || port[0] == '\0') {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    struct blob_buf bbuf;
    memset(&bbuf, 0, sizeof(bbuf));
    blob_buf_init(&bbuf, 0);
    blobmsg_add_string(&bbuf, "action", "off");
    blobmsg_add_u32(&bbuf, "pin", pin);
    blobmsg_add_string(&bbuf, "port", port);

    char* json = blobmsg_format_json_with_cb(bbuf.head, true, NULL, NULL, 0);
    int status;

    if (!json) {
        blob_buf_free(&bbuf);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    status = send_serial_request(ctx, req, port, json);
    blob_buf_free(&bbuf);
    free((void*)json);

    return status;
}

int get_method(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg)
{
    (void)ctx; (void)obj; (void)req; (void)method;
    struct blob_attr *tb[__MSG_MAX] = {0};

    if (!msg) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    blobmsg_parse(get_policy, ARRAY_SIZE(get_policy),
              tb,
              blob_data(msg),
              blob_len(msg));

    if (!tb[MSG_PIN] || !tb[MSG_PORT]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    int pin = blobmsg_get_u32(tb[MSG_PIN]);
    const char* model = tb[MSG_MODEL] ? blobmsg_get_string(tb[MSG_MODEL]) : "";
    const char* sensor = tb[MSG_SENSOR] ? blobmsg_get_string(tb[MSG_SENSOR]) : "";
    const char* port = blobmsg_get_string(tb[MSG_PORT]);
    if (!port || port[0] == '\0') {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    struct blob_buf bbuf;
    memset(&bbuf, 0, sizeof(bbuf));
    blob_buf_init(&bbuf, 0);
    blobmsg_add_string(&bbuf, "action", "get");
    blobmsg_add_u32(&bbuf, "pin", pin);
    blobmsg_add_string(&bbuf, "model", model);
    blobmsg_add_string(&bbuf, "sensor", sensor);
    blobmsg_add_string(&bbuf, "port", port);
    char* json = blobmsg_format_json_with_cb(bbuf.head, true, NULL, NULL, 0);
    int status;

    if (!json) {
        blob_buf_free(&bbuf);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    status = send_serial_request(ctx, req, port, json);
    blob_buf_free(&bbuf);
    free((void*)json);

    return status;
}

int devices_method(struct ubus_context *ctx, struct ubus_object *obj,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg)
{
    (void)ctx; (void)obj; (void)req; (void)method; (void)msg;
    char* json = NULL;
    if (get_device_json(&json) != 0) {
        return UBUS_STATUS_UNKNOWN_ERROR;
    }
    struct blob_buf reply_buf;
    memset(&reply_buf, 0, sizeof(reply_buf));
    blob_buf_init(&reply_buf, 0);
    if (!blobmsg_add_json_from_string(&reply_buf, json)) {
        blob_buf_free(&reply_buf);
        free(json);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }
    ubus_send_reply(ctx, req, reply_buf.head);
    blob_buf_free(&reply_buf);
    if (json) {
        free(json);
    }
    return 0;
}

const struct ubus_method my_methods[] = {
    UBUS_METHOD("on", on_method, on_policy),
    UBUS_METHOD("off", off_method, off_policy),
    UBUS_METHOD("get", get_method, get_policy),
    UBUS_METHOD_NOARG("devices", devices_method)
};

struct ubus_object_type my_object_type =
    UBUS_OBJECT_TYPE("esp_module", my_methods);

struct ubus_object my_object = {
    .name = "esp_module",
    .type = &my_object_type,
    .methods = my_methods,
    .n_methods = ARRAY_SIZE(my_methods),
};
