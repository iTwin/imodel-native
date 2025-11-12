#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#endif

#ifdef _WIN32
#  define socketerr WSAGetLastError()
#  define ssize_t   int
#else
#  define socketerr   errno
#  define SOCKET      int
#  define closesocket close
#endif

#include "log.h"
#include "util.h"

// Fetch proxy auto configuration using HTTP only
char *fetch_get(const char *url, int32_t *error) {
    struct addrinfo hints = {0};
    struct addrinfo *address_info = NULL;
    SOCKET sfd = 0;
    char *body = NULL;
    char *host = NULL;
    int32_t err = 0;
    ssize_t count = 0;

    if (!url)
        return NULL;

    // Check to make sure we are only using http:// urls
    if (strstr(url, "https://")) {
        err = ENOTSUP;
        log_error("HTTPS not supported (%" PRId32 ")", err);
        goto download_cleanup;
    }

    // Parse host name from url
    host = get_url_host(url);
    if (!host) {
        err = EADDRNOTAVAIL;
        log_error("Unable to parse URL host (%" PRId32 ")", err);
        goto download_cleanup;
    }

    // Parse port name from host
    char *port = strchr(host, ':');
    if (port) {
        *port = 0;
        port++;
    } else {
        port = "http";
    }

    // Attempt to resolve the host name
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    err = getaddrinfo(host, port, &hints, &address_info);
    if (err != 0) {
        err = socketerr;
        log_debug("Unable to resolve host %s (%" PRId32 ")", host, err);
        goto download_cleanup;
    }

    // Create communication socket
    sfd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if ((int)sfd == -1) {
        err = socketerr;
        log_error("Unable to create socket (%" PRId32 ")", err);
        goto download_cleanup;
    }

    // Connect to remote address
    err = connect(sfd, address_info->ai_addr, (int)address_info->ai_addrlen);
    if (err != 0) {
        err = socketerr;
        log_debug("Unable to connect to host %s (%" PRId32 ")", host, err);
        goto download_cleanup;
    }

    // Create http request using bare-minimum headers
    char request[512];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Accept: application/x-ns-proxy-autoconfig\r\n"
             "Connection: close\r\n"
             "\r\n",
             get_url_path(url), host);

    // Send request
    size_t written = send(sfd, request, (int)strlen(request), 0);
    if (written != strlen(request)) {
        err = socketerr;
        log_error("Unable to send HTTP request (%" PRId32 ")", err);
        goto download_cleanup;
    }

    char response[1024];
    int32_t response_len = 0;
    do {
        count = recv(sfd, response + response_len, sizeof(response) - response_len, 0);
        if (count <= 0)
            break;
        response_len += count;
        if (response_len == sizeof(response))
            break;
    } while (true);

    // Find the content length header
    const char *content_length_header = str_find_len_case_str(response, response_len, "Content-Length: ");
    if (!content_length_header) {
        err = EIO;
        log_error("Unable to find Content-Length header (%" PRId32 ")", err);
        goto download_cleanup;
    }

    // Parse the content length
    const size_t content_length = strtoul(content_length_header + 16, NULL, 0);
    if (content_length <= 0 || content_length >= SCRIPT_MAX) {
        err = EIO;
        log_error("Invalid Content-Length header (%" PRId64 "/%" PRId32 ")", content_length, err);
        goto download_cleanup;
    }

    // Create a buffer to hold the response body
    body = (char *)calloc(content_length + 1, sizeof(char));
    if (!body) {
        err = ENOMEM;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "response body", err);
        goto download_cleanup;
    }

    // Find the start of the response body
    const char *body_start = str_find_len_str(response, response_len, "\r\n\r\n");
    if (!body_start) {
        err = EIO;
        log_error("Unable to find response body (%" PRId32 ")", err);
        goto download_cleanup;
    }
    body_start += 4;

    // Copy the response body into the buffer
    const size_t response_header_len = (size_t)(body_start - response);
    size_t body_length = response_len - response_header_len;
    memcpy(body, body_start, body_length);

    // Read the remaining body
    if (content_length > body_length) {
        do {
            count = recv(sfd, body + body_length, (int)(content_length - body_length), 0);
            if (count <= 0)
                break;
            body_length += count;
            if (body_length == content_length)
                break;
        } while (1);
    }

    body[body_length] = 0;

download_cleanup:
    if (address_info)
        freeaddrinfo(address_info);
    if (sfd)
        closesocket(sfd);

    free(host);

    if (error)
        *error = err;

    if (err != 0) {
        free(body);
        return NULL;
    }

    return body;
}

bool fetch_global_init(void) {
    return true;
}

bool fetch_global_cleanup(void) {
    return true;
}
