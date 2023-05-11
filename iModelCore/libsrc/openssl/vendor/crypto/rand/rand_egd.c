/*
 * Copyright 2000-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/opensslconf.h>
#ifdef OPENSSL_NO_EGD
NON_EMPTY_TRANSLATION_UNIT
#else

# include <openssl/crypto.h>
# include <openssl/e_os2.h>
# include <openssl/rand.h>

/*
 * Query an EGD
 */

# if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_VMS) || defined(OPENSSL_SYS_MSDOS) || defined(OPENSSL_SYS_VXWORKS) || defined(OPENSSL_SYS_VOS) || defined(OPENSSL_SYS_UEFI)
int RAND_query_egd_bytes(const char *path, unsigned char *buf, int bytes)
{
    return -1;
}

int RAND_egd(const char *path)
{
    return -1;
}

int RAND_egd_bytes(const char *path, int bytes)
{
    return -1;
}

# else

#  include OPENSSL_UNISTD
#  include <stddef.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  ifndef NO_SYS_UN_H
#   ifdef OPENSSL_SYS_VXWORKS
#    include <streams/un.h>
#   else
#    include <sys/un.h>
#   endif
#  else
struct sockaddr_un {
    short sun_family;           /* AF_UNIX */
    char sun_path[108];         /* path name (gag) */
};
<<<<<<< HEAD
#  endif                         /* NO_SYS_UN_H */
#  include <string.h>
#  include <errno.h>
=======
# endif                         /* NO_SYS_UN_H */
# include <string.h>
# include <errno.h>

# if defined(OPENSSL_SYS_TANDEM)
/*
 * HPNS:
 *
 *  This code forces the use of compatibility mode if required on HPE NonStop
 *  when coreutils PRNGD is used and then restores the previous mode
 *  after establishing the socket. This is not required on x86 where hardware
 *  randomization should be used instead of EGD available as of OpenSSL 3.0.
 *  Use --with-rand-seed=rdcpu when configuring x86 with 3.0 and above.
 *
 *  Needs review:
 *
 *  The better long-term solution is to either run two EGD's each in one of
 *  the two modes or revise the EGD code to listen on two different sockets
 *  (each in one of the two modes) or use the hardware randomizer.
 */
_variable
int hpns_socket(int family,
                int type,
                int protocol,
                char* transport)
{
    int  socket_rc;
    char current_transport[20];

#  define AF_UNIX_PORTABILITY    "$ZAFN2"
#  define AF_UNIX_COMPATIBILITY  "$ZPLS"

    if (!_arg_present(transport) || transport == NULL || transport[0] == '\0')
        return socket(family, type, protocol);

    socket_transport_name_get(AF_UNIX, current_transport, 20);

    if (strcmp(current_transport,transport) == 0)
        return socket(family, type, protocol);

    /* set the requested socket transport */
    if (socket_transport_name_set(AF_UNIX, transport))
        return -1;

    socket_rc = socket(family,type,protocol);

    /* set mode back to what it was */
    if (socket_transport_name_set(AF_UNIX, current_transport))
        return -1;

    return socket_rc;
}

/*#define socket(a,b,c,...) hpns_socket(a,b,c,__VA_ARGS__) */

static int hpns_connect_attempt = 0;

# endif /* defined(OPENSSL_SYS_HPNS) */

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))

int RAND_query_egd_bytes(const char *path, unsigned char *buf, int bytes)
{
    FILE *fp = NULL;
    struct sockaddr_un addr;
    int mybuffer, ret = -1, i, numbytes, fd;
    unsigned char tempbuf[255];

    if (bytes > (int)sizeof(tempbuf))
        return -1;

    /* Make socket. */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (strlen(path) >= sizeof(addr.sun_path))
        return -1;
    strcpy(addr.sun_path, path);
    i = offsetof(struct sockaddr_un, sun_path) + strlen(path);
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1 || (fp = fdopen(fd, "r+")) == NULL)
        return -1;
    setbuf(fp, NULL);

    /* Try to connect */
    for ( ; ; ) {
        if (connect(fd, (struct sockaddr *)&addr, i) == 0)
            break;
#  ifdef EISCONN
        if (errno == EISCONN)
            break;
#  endif
        switch (errno) {
#  ifdef EINTR
        case EINTR:
#  endif
#  ifdef EAGAIN
        case EAGAIN:
#  endif
#  ifdef EINPROGRESS
        case EINPROGRESS:
#  endif
#  ifdef EALREADY
        case EALREADY:
#  endif
            /* No error, try again */
            break;
        default:
            ret = -1;
            goto err;
        }
    }

    /* Make request, see how many bytes we can get back. */
    tempbuf[0] = 1;
    tempbuf[1] = bytes;
    if (fwrite(tempbuf, sizeof(char), 2, fp) != 2 || fflush(fp) == EOF)
        goto err;
    if (fread(tempbuf, sizeof(char), 1, fp) != 1 || tempbuf[0] == 0)
        goto err;
    numbytes = tempbuf[0];

    /* Which buffer are we using? */
    mybuffer = buf == NULL;
    if (mybuffer)
        buf = tempbuf;

    /* Read bytes. */
    i = fread(buf, sizeof(char), numbytes, fp);
    if (i < numbytes)
        goto err;
    ret = numbytes;
    if (mybuffer)
        RAND_add(tempbuf, i, i);

 err:
    if (fp != NULL)
        fclose(fp);
    return ret;
}

int RAND_egd_bytes(const char *path, int bytes)
{
    int num;

    num = RAND_query_egd_bytes(path, NULL, bytes);
    if (num < 0)
        return -1;
    if (RAND_status() != 1)
        return -1;
    return num;
}

int RAND_egd(const char *path)
{
    return RAND_egd_bytes(path, 255);
}

# endif

#endif
