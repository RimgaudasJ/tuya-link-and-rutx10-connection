#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <net/if.h>

#define MAX_NET_IFACES 16

typedef struct {
    char     name[IF_NAMESIZE];
    char     ip[16];      /* dotted-decimal IPv4 */
    char     netmask[16]; /* dotted-decimal IPv4 */
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    int      is_up;
} net_iface_t;

int get_network_interfaces(net_iface_t *ifaces, int *count);

#endif /* NETWORK_H */
