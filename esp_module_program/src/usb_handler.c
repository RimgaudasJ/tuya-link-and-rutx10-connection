#include <stdio.h>
#include <stdlib.h>
#include <libserialport.h>
#include "include/usb_handler.h"
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <string.h>

#ifndef UBUS_STATUS_PARSE_ERROR
#define UBUS_STATUS_PARSE_ERROR UBUS_STATUS_INVALID_ARGUMENT
#endif

#define RETRY_COUNT 5

int setup_serial(struct sp_port **port) {
    if (!port) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    sp_set_baudrate(*port, 9600);
    sp_set_bits(*port, 8);
    sp_set_parity(*port, SP_PARITY_NONE);
    sp_set_stopbits(*port, 1);
    sp_set_flowcontrol(*port, SP_FLOWCONTROL_NONE);
    return UBUS_STATUS_OK;
}

int get_device_json(char** return_json) {
    struct sp_port **ports;

    int ret = sp_list_ports(&ports);
    if (ret != SP_OK) {
        return UBUS_STATUS_UNKNOWN_ERROR;
    }
    struct blob_buf bbuf;
    memset(&bbuf, 0, sizeof(bbuf));
    blob_buf_init(&bbuf, 0);
    struct blob_attr *devices =  blobmsg_open_array(&bbuf, "devices");
    for (int i = 0; ports[i] != NULL; i++) {
        const char* port_name = sp_get_port_name(ports[i]);
        int vid, pid;
        if (sp_get_port_usb_vid_pid(ports[i], &vid, &pid) != SP_OK) {
            continue;
        }
        if (port_name) {
            char device_str[128];

            snprintf(device_str, sizeof(device_str),
                    "port:%s, vid:%d, pid:%d",
                    port_name, vid, pid);

            blobmsg_add_string(&bbuf, NULL, device_str);
        }

    }
    blobmsg_close_array(&bbuf, devices);
    char* json = blobmsg_format_json_with_cb(bbuf.head, true, NULL, NULL, 0);
    
    if (json) {
        *return_json = strdup(json);
        if (!*return_json) {
            free((void*)json);
            return UBUS_STATUS_PARSE_ERROR;
        }
        free((void*)json);
    } else {
        *return_json = strdup("{}");
        if (!*return_json) {
            return UBUS_STATUS_PARSE_ERROR;
        }
    }
        
    blob_buf_free(&bbuf);
    sp_free_port_list(ports);
    return UBUS_STATUS_OK;
}

int read_data_from_port(struct sp_port *port, char** reply_json, int *got_reply) {
    if (!port) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    char buf[101];
    int bytes_read;

    bytes_read = sp_blocking_read(port, buf, sizeof(buf) - 1, 2000);

    if (bytes_read > 0) {
        *got_reply = 1;
        buf[bytes_read] = '\0';
        if (reply_json) {
            *reply_json = strdup(buf);
            if (!*reply_json) {
                return UBUS_STATUS_PARSE_ERROR;
            }
        }
        return UBUS_STATUS_OK;
    }

    if (bytes_read < 0) {
        return UBUS_STATUS_UNKNOWN_ERROR;
    }
    
    return UBUS_STATUS_OK;
}

int send_data_to_port(struct sp_port *port, const char* data) {
    if (!port || !data) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    size_t data_len = strlen(data);
    int bytes_written = sp_blocking_write(port, data, data_len, 2000);

    if (bytes_written < 0 || (size_t)bytes_written != data_len) {
        return UBUS_STATUS_UNKNOWN_ERROR;
    } 
    return UBUS_STATUS_OK;
}

int send_and_get_reply(const char* port_name, const char* data, char** reply) {
    struct sp_port *port = NULL;
    int ret = -1;

    if (!port_name || port_name[0] == '\0' || !data) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    // Get port by name
    if (sp_get_port_by_name(port_name, &port) != SP_OK) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    // Open port
    if (sp_open(port, SP_MODE_READ_WRITE) != SP_OK) {
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    if (setup_serial(&port) != UBUS_STATUS_OK) {
        goto cleanup;
    }
    
    int got_reply = 0;
    for (int attempt = 0; attempt < RETRY_COUNT; attempt++) {
        if (send_data_to_port(port, data) != UBUS_STATUS_OK) {
            goto cleanup;
        }

        if (read_data_from_port(port, reply, &got_reply) != UBUS_STATUS_OK) {
            goto cleanup;
        }
        if (got_reply) {
            break;
        }
        sleep(1);
    }
    ret = got_reply ? UBUS_STATUS_OK : UBUS_STATUS_UNKNOWN_ERROR;

cleanup:
    if (port) {
        sp_close(port);
        sp_free_port(port);
    }
    return ret;
}