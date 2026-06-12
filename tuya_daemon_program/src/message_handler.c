#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "include/message_handler.h"
#include "include/ubus_handler.h"

static int toggle_state = 0; // 0 for off, 1 for on

enum {
    ACTION_CODE,
    INPUT_PARAMS,
    __MAX
};

static const struct blobmsg_policy policy[__MAX] = {
    [ACTION_CODE] = { .name = "actionCode", .type = BLOBMSG_TYPE_STRING },
    [INPUT_PARAMS] = { .name = "inputParams", .type = BLOBMSG_TYPE_TABLE },
};


static void get_attr_from_data(struct blob_buf *buf, struct blob_attr *attr)
{
    int rem;
    struct blob_attr *pos;

    blobmsg_for_each_attr(pos, attr, rem) {
        const char *name = blobmsg_name(pos);

        if (name && strcmp(name, "data") == 0) {
            int rem2;
            struct blob_attr *child;
            blobmsg_for_each_attr(child, pos, rem2) {
                blobmsg_add_blob(buf, child);
            }
        } else {
            blobmsg_add_blob(buf, pos);
        }
    }
}


int parse_json_message(const char *json, char *action,struct blob_attr **input_params) {
    struct blob_buf b = {0};
    struct blob_attr *tb[__MAX];

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    if (!blobmsg_add_json_from_string(&b, json)) {
        syslog(LOG_WARNING, "Failed to parse JSON");
        blob_buf_free(&b);
        return -1;
    }

    blobmsg_parse(policy, __MAX, tb, blob_data(b.head), blob_len(b.head));

    if (!tb[ACTION_CODE]) {
        syslog(LOG_WARNING, "Key 'actionCode' not found");
        blob_buf_free(&b);
        return -1;
    }

    const char *value = blobmsg_get_string(tb[ACTION_CODE]);
    snprintf(action, 64, "%s", value);
    syslog(LOG_INFO, "Parsed action: %s", value);

    if (input_params && tb[INPUT_PARAMS]) {
        *input_params = blob_memdup(tb[INPUT_PARAMS]);
    }

    blob_buf_free(&b);
    return 0;
}

int handle_ubus_method(const char* action,struct blob_attr *params, struct blob_attr **response) {
    char* ubus_method;
    if(strcmp(action, "action_get_sensor_data") == 0){
        ubus_method = "get";
        syslog(LOG_INFO, "Handling get_sensor_data action");
    } else if (strcmp(action, "action_get_devices") == 0) {
        ubus_method = "devices";
        syslog(LOG_INFO, "Handling get_devices action");
    } else if (strcmp(action, "action_toggle") == 0) {
        toggle_state = !toggle_state;
        ubus_method = toggle_state ? "on" : "off";
        syslog(LOG_INFO, "Handling toggle action, new state: %d", toggle_state);
    } else {
        syslog(LOG_WARNING, "Unknown action '%s'", action);
        return -1;
    }

    int ret = ubus_call_method(ubus_method, params, response);

    if (ret != 0) {
        syslog(LOG_ERR, "Failed to call ubus method '%s'", ubus_method);
        return -1;
    }
    syslog(LOG_DEBUG, "Successfully called ubus method '%s'", ubus_method);
    // Process response if needed
    if (response && *response) { 
        char *str = blobmsg_format_json(*response, true);
        syslog(LOG_INFO, "ubus method '%s' response:\n%s\n", ubus_method, str);
        free(str);
    }
    return 0;
}


int handle_message(const char* message, char **json_response) {
    char action[64];
    struct blob_attr *input_params = NULL;
    int ret;

    if (parse_json_message(message, action, &input_params) != 0) {
        syslog(LOG_WARNING, "Failed to parse message");
        return -1;
    }

    struct blob_attr *response = NULL;
    ret = handle_ubus_method(action, input_params, &response);
    if(input_params) {
        free(input_params);
    }
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to handle ubus method for action '%s'", action);
        return -1;
    }
    if (response) {
        struct blob_buf b;
        memset(&b, 0, sizeof(b));
        blob_buf_init(&b, 0);
        get_attr_from_data(&b, response);
        *json_response = blobmsg_format_json(b.head, true);
        blob_buf_free(&b);
        free(response);
    }
    return 0;
}


