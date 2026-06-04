#include "option_parse.h"
#include <argp.h>

const struct argp_option options[] = {
    { "device-id",     'd', "DEVICE_ID",     0, "Tuya device ID",     0 },
    { "device-secret", 's', "DEVICE_SECRET", 0, "Tuya device secret", 0 },
    { "product-id",    'p', "PRODUCT_ID",    0, "Tuya product ID",    0 },
    { "daemon",        'D', NULL,            0, "Run as daemon",      0 },
    { 0, 0, 0, 0, 0, 0 }
};

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;
    switch (key) {
        case 'd': args->device_id     = arg; break;
        case 's': args->device_secret = arg; break;
        case 'p': args->product_id    = arg; break;
        case 'D': args->daemon_flag   = 1;   break;
        case ARGP_KEY_END:
            if (args->device_id == NULL)
                argp_error(state, "required option --device-id missing");
            if (args->device_secret == NULL)
                argp_error(state, "required option --device-secret missing");
            if (args->product_id == NULL)
                argp_error(state, "required option --product-id missing");
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
