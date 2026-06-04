#define _POSIX_C_SOURCE 200809L
#include "include/ubus_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>




int main(void){  
    struct ubus_context *ctx;

     if (uloop_init() != 0) {
        syslog(LOG_ERR, "Failed to initialize uloop");
        return UBUS_STATUS_UNKNOWN_ERROR ;
    }

    ctx = ubus_connect(NULL);
    if (!ctx) {
        syslog(LOG_ERR, "Failed to connect to ubus");
        uloop_done();
        return UBUS_STATUS_CONNECTION_FAILED;
    }

    ubus_add_uloop(ctx);
    int ret = ubus_add_object(ctx, &my_object);
    if (ret) {
        syslog(LOG_ERR, "Failed to add object: %s", ubus_strerror(ret));
        ubus_free(ctx);
        uloop_done();
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    uloop_run();

    ubus_free(ctx);
    uloop_done();

    return 0;
}