#include "json_builder.h"
#include "hardware.h"
#include "network.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>

static int appendf(char *buf, size_t len, const char *fmt, ...)
{
    size_t used;
    va_list ap;
    int written;

    if (!buf || !fmt || len == 0)
        return -1;

    used = strnlen(buf, len);
    if (used >= len)
        return -1;

    va_start(ap, fmt);
    written = vsnprintf(buf + used, len - used, fmt, ap);
    va_end(ap);

    if (written < 0 || (size_t)written >= (len - used))
        return -1;

    return 0;
}

int build_json_ram_and_uptime(char *buf, size_t len)
{
    system_metrics_t hw;
    if (get_system_metrics(&hw) != 0) {
        syslog(LOG_ERR, "get_system_metrics failed");
        hw.total_ram = hw.free_ram = hw.uptime_secs = 0;
    }

    if (appendf(buf, len,
                "\"ram\":\"total: %llu, free: %llu\","
                "\"uptime\": \"%llu\",",
                (unsigned long long)hw.total_ram,
                (unsigned long long)hw.free_ram,
                (unsigned long long)hw.uptime_secs) != 0) {
        syslog(LOG_ERR, "JSON buffer overflow while adding RAM/uptime");
        return -1;
    }

    return 0;
}

int build_json_cpu(char *buf, size_t len)
{
    int cpu_pct = 0;
    if (get_cpu_usage(&cpu_pct) != 0) {
        syslog(LOG_ERR, "get_cpu_usage failed");
        cpu_pct = 0;
    }

    if (appendf(buf, len, "\"cpu_usage\": \"%d\",", cpu_pct) != 0) {
        syslog(LOG_ERR, "JSON buffer overflow while adding CPU");
        return -1;
    }

    return 0;
}

int build_json_network(char *buf, size_t len)
{
    net_iface_t ifaces[MAX_NET_IFACES];
    int iface_count = 0;
    if (get_network_interfaces(ifaces, &iface_count) != 0) {
        syslog(LOG_ERR, "get_network_interfaces failed");
        iface_count = 0;
    }

    char net_buf[NET_BUF];
    net_buf[0] = '\0';

    if (appendf(net_buf, sizeof(net_buf), "\"interfaces\": [") != 0) {
        syslog(LOG_ERR, "JSON buffer overflow while starting interfaces block");
        return -1;
    }

    for (int i = 0; i < iface_count; i++) {
        if (appendf(net_buf, sizeof(net_buf),
                    "\"name: %s, ip: %s, netmask: %s, "
                    "tx_bytes: %llu, rx_bytes: %llu\"%s",
                    ifaces[i].name,
                    ifaces[i].ip,
                    ifaces[i].netmask,
                    (unsigned long long)ifaces[i].tx_bytes,
                    (unsigned long long)ifaces[i].rx_bytes,
                    (i < iface_count - 1) ? "," : "") != 0) {
            syslog(LOG_ERR, "JSON buffer overflow while adding interface %d", i);
            return -1;
        }
    }

    if (appendf(net_buf, sizeof(net_buf), "]") != 0) {
        syslog(LOG_ERR, "JSON buffer overflow while finishing interfaces block");
        return -1;
    }

    if (appendf(buf, len, "%s", net_buf) != 0) {
        syslog(LOG_ERR, "JSON buffer overflow while appending interfaces block");
        return -1;
    }

    return 0;
}


int build_json(char *buf, size_t len)
{
    int ret;
    /* Build JSON string */
    syslog(LOG_INFO, "Building JSON data");
    snprintf(buf, len, "{");
    syslog(LOG_INFO, "Adding RAM and uptime info to JSON");
    ret = build_json_ram_and_uptime(buf, len);
    if (ret != 0) return ret;
    syslog(LOG_INFO, "Adding CPU info to JSON");
    ret = build_json_cpu(buf, len);
    if (ret != 0) return ret;
    syslog(LOG_INFO, "Adding network info to JSON");
    ret = build_json_network(buf, len);
    if (ret != 0) return ret;
    if (appendf(buf, len, "}") != 0) {
        syslog(LOG_ERR, "JSON buffer overflow while finalizing JSON");
        return -1;
    }

    syslog(LOG_INFO, "Finished building JSON data");
    return 0;
}