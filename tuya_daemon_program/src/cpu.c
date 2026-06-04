#include "cpu.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
} cpu_stat_t;

static int read_cpu_stat(cpu_stat_t *s)
{
    FILE *f = fopen("/proc/stat", "r");
    int ret;
    if (f == NULL)
        return -1;
    ret = fscanf(f, "cpu %llu %llu %llu %llu %llu %llu %llu",
                 &s->user, &s->nice, &s->system, &s->idle,
                 &s->iowait, &s->irq, &s->softirq);
    fclose(f);
    return (ret == 7) ? 0 : -1;
}

int get_cpu_usage(int *percent)
{
    cpu_stat_t a, b;
    unsigned long long total_a, total_b, idle_a, idle_b;
    unsigned long long total_delta, idle_delta;

    if (percent == NULL)
        return EINVAL;

    if (read_cpu_stat(&a) != 0)
        return errno;

    sleep(2); /* 2 seconds */

    if (read_cpu_stat(&b) != 0)
        return errno;

    idle_a  = a.idle + a.iowait;
    idle_b  = b.idle + b.iowait;
    total_a = a.user + a.nice + a.system + a.idle + a.iowait + a.irq + a.softirq;
    total_b = b.user + b.nice + b.system + b.idle + b.iowait + b.irq + b.softirq;

    total_delta = total_b - total_a;
    idle_delta  = idle_b  - idle_a;

    if (total_delta == 0) {
        *percent = 0;
    } else {
        *percent = (int)(100ULL * (total_delta - idle_delta) / total_delta);
    }

    return 0;
}
