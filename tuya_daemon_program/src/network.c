#include "network.h"
#include <uci.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <syslog.h>
#include <arpa/inet.h>

static struct ubus_context *ctx;


static const struct blobmsg_policy policy[] = {
    { .name = "up", .type = BLOBMSG_TYPE_BOOL },
    { .name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY },
    { .name = "device", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy ipv4_policy[] = {
    { .name = "address", .type = BLOBMSG_TYPE_STRING },
    { .name = "mask", .type = BLOBMSG_TYPE_INT32 },
};

static const struct blobmsg_policy iface_policy[] = {
    { .name = "tx_bytes", .type = BLOBMSG_TYPE_INT64 },
    { .name = "rx_bytes", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy dev_policy[] = {
    { .name = "statistics", .type = BLOBMSG_TYPE_TABLE },
    { .name = "tx_bytes", .type = BLOBMSG_TYPE_INT64 },
    { .name = "rx_bytes", .type = BLOBMSG_TYPE_INT64 },
};

static void prefix_to_netmask(int prefix, char *out, size_t out_len)
{
    struct in_addr addr;
    uint32_t mask;

    if (prefix < 0 || prefix > 32 || out_len == 0) {
        if (out_len > 0)
            out[0] = '\0';
        return;
    }

    mask = (prefix == 0) ? 0 : (0xFFFFFFFFu << (32 - prefix));
    addr.s_addr = htonl(mask);

    if (inet_ntop(AF_INET, &addr, out, out_len) == NULL)
        out[0] = '\0';
}

static void dev_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    net_iface_t *iface = req ? (net_iface_t *)req->priv : NULL;

    if (!iface || !msg) {
        syslog(LOG_ERR, "Invalid argument to dev_cb");
        return;
    }

    if (type != UBUS_MSG_DATA) {
        syslog(LOG_ERR, "unexpected ubus message type: %d", type);
        return;
    }

    struct blob_attr *tb[3] = { 0 };
    struct blob_attr *stats_tb[2] = { 0 };
    int have_stats = 0;

    blobmsg_parse(dev_policy, 3, tb, blob_data(msg), blob_len(msg));

    if (tb[0]) {
        blobmsg_parse(iface_policy, 2, stats_tb, blobmsg_data(tb[0]), blobmsg_len(tb[0]));

        if (stats_tb[0]) {
            iface->tx_bytes = blobmsg_get_u64(stats_tb[0]);
            have_stats = 1;
        }

        if (stats_tb[1]) {
            iface->rx_bytes = blobmsg_get_u64(stats_tb[1]);
            have_stats = 1;
        }
    }

    if (!have_stats) {
        if (tb[1]) {
            iface->tx_bytes = blobmsg_get_u64(tb[1]);
            have_stats = 1;
        }

        if (tb[2]) {
            iface->rx_bytes = blobmsg_get_u64(tb[2]);
            have_stats = 1;
        }
    }

    (void)have_stats;
}

static void ubus_parse_msg(net_iface_t *iface, struct blob_attr *msg)
{
    if(!iface || !msg) {
        syslog(LOG_ERR, "Invalid argument to ubus_parse_msg");
        return;
    }
    struct blob_attr *tb[3] = { 0 };

    blobmsg_parse(policy, 3, tb, blob_data(msg), blob_len(msg));

    // up
    if (tb[0]) {
        iface->is_up = blobmsg_get_bool(tb[0]);
    }

    // ipv4-address
    if (tb[1]) {
        struct blob_attr *entry;
        int rem;

        blobmsg_for_each_attr(entry, tb[1], rem) {
            struct blob_attr *ip_tb[2] = { 0 };

            if (blobmsg_type(entry) != BLOBMSG_TYPE_TABLE)
                continue;

            blobmsg_parse(ipv4_policy, 2, ip_tb, blobmsg_data(entry), blobmsg_len(entry));

            if (ip_tb[0]) {
                strncpy(iface->ip, blobmsg_get_string(ip_tb[0]), sizeof(iface->ip) - 1);
                iface->ip[sizeof(iface->ip) - 1] = '\0';
            }

            if (ip_tb[1]) {
                prefix_to_netmask(blobmsg_get_u32(ip_tb[1]), iface->netmask, sizeof(iface->netmask));
            }

            break;
        }
    }

    // device
    if (tb[2]) {
        
        struct blob_buf dev_buf;
        
       const char *dev_name = blobmsg_get_string(tb[2]);
        char dev_name_copy[IF_NAMESIZE];

        if (!dev_name || dev_name[0] == '\0')
            return;

        strncpy(dev_name_copy, dev_name, sizeof(dev_name_copy) - 1);
        dev_name_copy[sizeof(dev_name_copy) - 1] = '\0';

        if(ctx == NULL) {
            syslog(LOG_ERR, "ubus context is NULL");
            return;
        }
        uint32_t dev_id;
        if (ubus_lookup_id(ctx, "network.device", &dev_id) != 0) {
            syslog(LOG_ERR, "Failed to lookup ubus ID for network.device");
        } else {
            memset(&dev_buf, 0, sizeof(dev_buf));
            blob_buf_init(&dev_buf, 0);
            if (blobmsg_add_string(&dev_buf, "name", dev_name_copy) != 0) {
                syslog(LOG_ERR, "Failed to add name to buffer");
                blob_buf_free(&dev_buf);
                return;
            }

            
            int ret = ubus_invoke(ctx, dev_id, "status", dev_buf.head, dev_cb, iface, 5000);

            if (ret != 0) {
                syslog(LOG_ERR, "ubus_invoke failed: %s (%d)", ubus_strerror(ret), ret);
            }

            blob_buf_free(&dev_buf);
        }
    }
}


static void ubus_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    net_iface_t *iface = req ? (net_iface_t *)req->priv : NULL;

    if (!iface)
        return;

    if (type != UBUS_MSG_DATA) {
        syslog(LOG_ERR, "unexpected ubus message type: %d", type);
        return;
    }

    ubus_parse_msg(iface, msg);
}


int uci_get_interfaces(char* i_faces[], int *count) {
    struct uci_context *ctx = uci_alloc_context();
    int err = 0;

    if (!ctx) {
        syslog(LOG_ERR, "Failed to allocate UCI context");
        return -1;
    }

    struct uci_package *pkg = NULL;

    if (uci_load(ctx, "network", &pkg) != UCI_OK) {
        syslog(LOG_ERR, "Failed to load UCI package");
        err = -1;
        goto cleanup;
    }

    struct uci_element *e;
    uci_foreach_element(&pkg->sections, e) {
        struct uci_section *s = uci_to_section(e);
        if (*count >= MAX_NET_IFACES)
            break;

        if (strcmp(s->type, "interface") == 0) {
            if (strcmp(s->e.name, "loopback") == 0)
                continue;

            i_faces[*count] = strdup(s->e.name);
            if (!i_faces[*count]) {
                err = -1;
                break;
            }
            (*count)++;
        }
    }

cleanup:
    uci_free_context(ctx);
    return err;
}


int get_network_interfaces(net_iface_t *ifaces, int *count) {
    char* i_face_names[MAX_NET_IFACES] = { 0 };
    int iface_count = 0;
    int err = 0;

    if (!ifaces || !count) {
        syslog(LOG_ERR, "Invalid argument");
        return -1;
    }

    *count = 0;

    if (uci_get_interfaces(i_face_names, &iface_count) != 0) {
        syslog(LOG_ERR, "Failed to get network interfaces");
        err = -1;
        goto cleanup_iface;
    }

    for (int i = 0; i < iface_count; i++) {
        memset(&ifaces[i], 0, sizeof(ifaces[i]));
        if (i_face_names[i]) {
            strncpy(ifaces[i].name, i_face_names[i], sizeof(ifaces[i].name) - 1);
            ifaces[i].name[sizeof(ifaces[i].name) - 1] = '\0';
        }
    }

    ctx = ubus_connect(NULL);
    if (!ctx) {
        syslog(LOG_ERR, "Failed to connect to ubus");
        err = -1;
        goto cleanup_iface;
    }

    for (int i = 0; i < iface_count; i++) {
        char obj[64];
        uint32_t id;

        if (snprintf(obj, sizeof(obj), "network.interface.%s", ifaces[i].name) >= (int)sizeof(obj)) {
            syslog(LOG_ERR, "ubus object path too long for iface: %s", ifaces[i].name);
            continue;
        }

        if (ubus_lookup_id(ctx, obj, &id) != 0) {
            syslog(LOG_ERR, "Failed to lookup ubus ID for %s", obj);
            continue;
        }

        if (ubus_invoke(ctx, id, "status", NULL, ubus_cb, &ifaces[i], 5000) != 0)
            syslog(LOG_ERR, "Failed to invoke ubus status for %s", obj);
    }

    *count = iface_count;

    ubus_free(ctx);
    ctx = NULL;
cleanup_iface:
    for (int i = 0; i < iface_count; i++) {
        if(i_face_names[i] != NULL)
            free(i_face_names[i]);
    }

    return err;
}