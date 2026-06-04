#ifndef TUYA_H
#define TUYA_H

#include <stddef.h>

#include "tuya_link/tuyalink_core.h"

int tuya_init(tuya_mqtt_context_t *client,
              const char *device_id,
              const char *device_secret);

int tuya_report(tuya_mqtt_context_t *client, const char *json);

int tuya_get_action(char *buffer, size_t buffer_len);

#endif /* TUYA_H */