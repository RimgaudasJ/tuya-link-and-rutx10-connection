#ifndef UBUS_HANDLER_H
#define UBUS_HANDLER_H

int ubus_call_method(const char *method,struct blob_attr *params,struct blob_attr **response);
int ubus_init_connection(struct ubus_context **ctx);

#endif /* UBUS_HANDLER_H */