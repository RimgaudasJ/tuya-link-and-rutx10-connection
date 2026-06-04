// for logging strings
#include <stdio.h>
#include <syslog.h>
#include "cjson/cJSON.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

int parse_from_json(const char* json_str, char* data_name, char* output_buf, size_t buf_size) {
    cJSON* json = cJSON_Parse(json_str);
    if (json == NULL) {
        syslog(LOG_ERR, "Failed to parse JSON");
        return -1;
    }

    cJSON* value = cJSON_GetObjectItem(json, "inputParams");
    value = cJSON_GetObjectItem(value, data_name);
    if (cJSON_IsString(value) && (value->valuestring != NULL)) {
        snprintf(output_buf, buf_size, "%s", value->valuestring);
        syslog(LOG_INFO, "Parsed value: %s", value->valuestring);
    } else {
        syslog(LOG_WARNING, "Key '%s' not found or not a string", data_name);
    }

    cJSON_Delete(json);
    return 0;
}

int log_message(const char* message, char* data_name) {
    char log_buf[512];
    mkdir("./tmp", 0777);
    FILE* f = fopen("./tmp/tuya_action.log", "a");
    if (f == NULL) {
        syslog(LOG_ERR, "Failed to open log file");
        return -1;
    }

    if (parse_from_json(message, data_name, log_buf, sizeof(log_buf)) != 0) {
        fclose(f);
        return -1;
    }
    fprintf(f, "%s\n", log_buf);
    fclose(f);
    return 0;
}