#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <windows.h>
#else
#  include <arpa/inet.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#endif

#include "log.h"
#include "net_adapter.h"
#include "util.h"
#include "wpad_dhcp_posix.h"

#ifdef _WIN32
#  define socketerr WSAGetLastError()
#  define ssize_t   int
#else
#  define socketerr   errno
#  define SOCKET      int
#  define closesocket close
#endif

#define DHCP_SERVER_PORT    (67)
#define DHCP_CLIENT_PORT    (68)

#define DHCP_MAGIC          ("\x63\x82\x53\x63")
#define DHCP_MAGIC_LEN      (4)

#define DHCP_ACK            (5)
#define DHCP_INFORM         (8)

#define DHCP_BOOT_REQUEST   (1)
#define DHCP_BOOT_REPLY     (2)

#define DHCP_OPT_PAD        (0)
#define DHCP_OPT_MSGTYPE    (0x35)
#define DHCP_OPT_PARAMREQ   (0x37)
#define DHCP_OPT_WPAD       (0xfc)
#define DHCP_OPT_END        (0xff)

#define DHCP_OPT_MIN_LENGTH (312)

#define ETHERNET_TYPE       (1)
#define ETHERNET_LENGTH     (6)

typedef struct dhcp_msg {
    uint8_t op;         /* operation */
    uint8_t htype;      /* hardware address type */
    uint8_t hlen;       /* hardware address len */
    uint8_t hops;       /* message hops */
    uint32_t xid;       /* transaction id */
    uint16_t secs;      /* seconds since protocol start */
    uint16_t flags;     /* 0 = unicast, 1 = broadcast */
    uint32_t ciaddr;    /* client IP */
    uint32_t yiaddr;    /* your IP */
    uint32_t siaddr;    /* server IP */
    uint32_t giaddr;    /* gateway IP */
    uint8_t chaddr[16]; /* client hardware address */
    uint8_t sname[64];  /* server name */
    uint8_t file[128];  /* bootstrap file */
    uint8_t options[DHCP_OPT_MIN_LENGTH];
} dhcp_msg;

typedef struct dhcp_option {
    uint8_t type;
    uint8_t length;
    uint8_t value[1];
} dhcp_option;

static inline bool dhcp_check_magic(uint8_t *options) {
    return memcmp(options, DHCP_MAGIC, DHCP_MAGIC_LEN) == 0;
}

static inline uint8_t *dhcp_copy_magic(uint8_t *options) {
    memcpy(options, DHCP_MAGIC, DHCP_MAGIC_LEN);
    return options + DHCP_MAGIC_LEN;
}

static inline uint8_t *dhcp_copy_option(uint8_t *options, dhcp_option *option) {
    memcpy(options, &option->type, sizeof(option->type));
    options += sizeof(option->type);
    memcpy(options, &option->length, sizeof(option->length));
    options += sizeof(option->length);
    if (option->length) {
        memcpy(options, option->value, option->length);
        options += option->length;
    }
    return options;
}

static uint8_t *dhcp_get_option(dhcp_msg *reply, uint8_t type, uint8_t *length) {
    uint8_t *opts = reply->options + DHCP_MAGIC_LEN;
    uint8_t *opts_end = reply->options + sizeof(reply->options);

    // Enumerate DHCP options
    while (opts < opts_end && *opts != DHCP_OPT_END) {
        if (*opts == DHCP_OPT_PAD) {
            opts++;
            continue;
        }

        // Parse option type and length
        uint8_t opt_type = *opts++;
        uint8_t opt_length = *opts++;

        // Check if option type matches
        if (opt_type == type) {
            // Allocate buffer to return option value
            uint8_t *value = (uint8_t *)calloc(opt_length + 1, sizeof(uint8_t));
            if (value)
                memcpy(value, opts, opt_length);

            // Optionally return option length
            if (length)
                *length = opt_length;

            return value;
        }

        opts += opt_length;
    }

    return NULL;
}

static bool dhcp_send_inform(SOCKET sfd, uint32_t xid, net_adapter_s *adapter) {
    struct sockaddr_in address = {0};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_NONE;
    address.sin_port = htons(DHCP_SERVER_PORT);

    // Construct request
    struct dhcp_msg request = {0};

    request.op = DHCP_BOOT_REQUEST;
    request.htype = ETHERNET_TYPE;
    request.hlen = adapter->mac_length;
    if (request.hlen > sizeof(request.chaddr))
        request.hlen = sizeof(request.chaddr);
    memcpy(request.chaddr, adapter->mac, request.hlen);
    request.xid = xid;
    request.ciaddr = *(uint32_t *)adapter->ip;
    request.yiaddr = *(uint32_t *)adapter->ip;
    request.siaddr = *(uint32_t *)adapter->dhcp;

    uint8_t *opts = request.options;

    // Construct request signature
    opts = dhcp_copy_magic(opts);

    // Construct request options
    dhcp_option opt_msg_type = {DHCP_OPT_MSGTYPE, 1, {DHCP_INFORM}};
    opts = dhcp_copy_option(opts, &opt_msg_type);
    dhcp_option opt_param_req = {DHCP_OPT_PARAMREQ, 1, {DHCP_OPT_WPAD}};
    opts = dhcp_copy_option(opts, &opt_param_req);
    dhcp_option opt_end = {DHCP_OPT_END, 0, {0}};
    opts = dhcp_copy_option(opts, &opt_end);

    // Broadcast DHCP request
    const ssize_t request_len = (ssize_t)(opts - (uint8_t *)&request);
    const ssize_t sent =
        sendto(sfd, (const char *)&request, request_len, 0, (struct sockaddr *)&address, sizeof(address));
    return sent == request_len;
}

static bool dhcp_read_reply(SOCKET sfd, uint32_t request_xid, dhcp_msg *reply) {
    const ssize_t response_len = recvfrom(sfd, (char *)reply, sizeof(dhcp_msg), 0, NULL, NULL);

    if (response_len <= (ssize_t)(sizeof(dhcp_msg) - DHCP_OPT_MIN_LENGTH)) {
        log_debug("Unable to read DHCP reply (%d:%d)", (int32_t)response_len, socketerr);
        return false;
    }

    if (reply->op != DHCP_BOOT_REPLY) {
        log_debug("Invalid DHCP reply operation (%" PRId32 ")", (int32_t)reply->op);
        return false;
    }

    if (reply->xid != request_xid) {
        log_error("Invalid DHCP reply transaction id (%" PRIx32 ")", reply->xid);
        return false;
    }

    if (!dhcp_check_magic(reply->options)) {
        log_error("Invalid DHCP reply magic (%" PRIx32 ")", *(uint32_t *)reply->options);
        return false;
    }
    return true;
}

char *wpad_dhcp_adapter_posix(uint8_t bind_ip[4], net_adapter_s *adapter, int32_t timeout_sec) {
    SOCKET sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ((int)sfd == -1) {
        log_error("Unable to create udp socket");
        return NULL;
    }

    int broadcast = 1;
    setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof(broadcast));
    int reuseaddr = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseaddr, sizeof(reuseaddr));
    struct timeval tv = {timeout_sec, 0};
    setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    struct sockaddr_in address = {0};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = *(uint32_t *)bind_ip;
    address.sin_port = htons(DHCP_CLIENT_PORT);

    int err = bind(sfd, (struct sockaddr *)&address, sizeof(address));
    if (err == -1) {
        // Likely can't bind to protected port, try again with random port
        if (socketerr == EACCES) {
            address.sin_port = 0;
            err = bind(sfd, (struct sockaddr *)&address, sizeof(address));
        }
        if (err == -1) {
            log_debug("Unable to bind udp socket (%d)", socketerr);
            closesocket(sfd);
            return NULL;
        }
    }

    // Generate random transaction id
    srand((int)time(NULL));
    uint32_t request_xid = rand();

    // Send DHCPINFORM request to DHCP server
    if (!dhcp_send_inform(sfd, request_xid, adapter)) {
        log_error("Unable to send DHCP inform");
        closesocket(sfd);
        return NULL;
    }

    // Read reply from DHCP server
    dhcp_msg reply = {0};
    bool is_ok = dhcp_read_reply(sfd, request_xid, &reply);
    closesocket(sfd);
    if (!is_ok)
        return NULL;

    // Parse options in DHCP reply
    uint8_t opt_length = 0;
    uint8_t *opt = NULL;

    opt = dhcp_get_option(&reply, DHCP_OPT_MSGTYPE, &opt_length);
    if (opt_length != 1 || *opt != DHCP_ACK) {
        log_error("Invalid DHCP reply (msgtype=%d)", *opt);
        return NULL;
    }
    free(opt);

    opt = dhcp_get_option(&reply, DHCP_OPT_WPAD, &opt_length);
    if (opt_length <= 0) {
        log_error("Invalid DHCP reply (optlen=%d)", opt_length);
        return NULL;
    }

    return (char *)opt;
}
