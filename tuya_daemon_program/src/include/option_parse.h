#ifndef OPTION_PARSE_H
#define OPTION_PARSE_H

#include <argp.h>
#include <stdlib.h>

extern const struct argp_option options[];

struct arguments {
    const char *device_id;
    const char *device_secret;
    const char *product_id;
    int         daemon_flag;
};

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif /* OPTION_PARSE_H */