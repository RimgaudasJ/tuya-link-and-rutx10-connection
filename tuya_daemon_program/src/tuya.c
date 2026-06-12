#include "tuya.h"
#include "tuya_link/tuya_cacert.h"
#include "tuya_link/tuya_error_code.h"
#include "tuya_link/tuya_log.h"
#include "tuya_link/tuyalink_core.h"
#include <stdio.h>
#include <syslog.h>
#include "message_handler.h"
#include<unistd.h>
#include <stdlib.h>

#define TUYA_ACTION_BUFFER_SIZE 512

static void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
    (void)context;
    (void)user_data;
    syslog(LOG_INFO, "Tuya MQTT connected");
}

static void on_disconnect(tuya_mqtt_context_t *context, void *user_data)
{
    (void)context;
    (void)user_data;
    syslog(LOG_WARNING, "Tuya MQTT disconnected");
}

static void on_messages(tuya_mqtt_context_t *context,
                        void *user_data,
                        const tuyalink_message_t *msg)
{
    (void)context;
    (void)user_data;

    if (!msg) {
        syslog(LOG_WARNING, "Received null Tuya message");
        return;
    }

    syslog(LOG_INFO,
           "Received message: topic=%s, data=%s",
           msg->device_id ? msg->device_id : "",
           msg->data_string ? msg->data_string : "");
    
    char *json_response = NULL;
    if (msg->data_string)
        handle_message(msg->data_string, &json_response);

    if (!json_response) {
        json_response = strdup("{}");
    }
    tuya_report(context, json_response);
    free(json_response);
}

int tuya_init(tuya_mqtt_context_t *client,
              const char *device_id,
              const char *device_secret)
{
    int ret;
    tuya_mqtt_config_t cfg;

    if (client == NULL || device_id == NULL || device_secret == NULL)
        return -1;

    cfg.host         = "m1.tuyacn.com";
    cfg.port         = 8883;
    cfg.cacert       = (const uint8_t *)tuya_cacert_pem;
    cfg.cacert_len   = sizeof(tuya_cacert_pem);
    cfg.client_cert  = NULL;
    cfg.client_cert_len = 0;
    cfg.client_key   = NULL;
    cfg.client_key_len  = 0;
    cfg.device_id    = device_id;
    cfg.device_secret = device_secret;
    cfg.keepalive    = 100;
    cfg.timeout_ms   = 2000;
    cfg.user_data    = NULL;
    cfg.on_connected = on_connected;
    cfg.on_disconnect = on_disconnect;
    cfg.on_messages  = on_messages;

    ret = tuya_mqtt_init(client, &cfg);
    if (ret != OPRT_OK) {
        syslog(LOG_ERR, "tuya_mqtt_init failed: %d", ret);
        return ret;
    }

    ret = tuya_mqtt_connect(client);
    if (ret != OPRT_OK) {
        syslog(LOG_ERR, "tuya_mqtt_connect failed: %d", ret);
        return ret;
    }

    return 0;
}

int tuya_report(tuya_mqtt_context_t *client, const char *json)
{
    
    if (!client || !json)
            return -1;

    int ret = tuyalink_thing_property_report(client, client->config.device_id, json);
    return ret;
}
