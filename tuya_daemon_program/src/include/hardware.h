#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

typedef struct {
    uint64_t total_ram;   /* bytes */
    uint64_t free_ram;    /* bytes */
    uint32_t uptime_secs; /* seconds since boot */
} system_metrics_t;

int get_system_metrics(system_metrics_t *out);

#endif /* HARDWARE_H */