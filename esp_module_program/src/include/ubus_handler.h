#ifndef UBUS_HANDLER_H
#define UBUS_HANDLER_H
#include <libubus.h>
#include <libubox/blobmsg_json.h>

enum {
    MSG_PIN,
    MSG_PORT,
    MSG_MODEL,
    MSG_SENSOR,
    __MSG_MAX
};

extern const struct blobmsg_policy on_policy[];
extern const struct blobmsg_policy off_policy[];
extern const struct blobmsg_policy get_policy[];

int on_method(struct ubus_context *ctx, struct ubus_object *obj,
                     struct ubus_request_data *req, const char *method,
                     struct blob_attr *msg);
int off_method(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg);
int get_method(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg);
int devices_method(struct ubus_context *ctx, struct ubus_object *obj,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg);

extern const struct ubus_method my_methods[];
extern struct ubus_object_type my_object_type;
extern struct ubus_object my_object;

#endif // UBUS_HANDLER_H