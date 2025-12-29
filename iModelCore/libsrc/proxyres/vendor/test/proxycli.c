#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "proxyres/config.h"
#include "proxyres/execute.h"
#include "proxyres/resolver.h"

#ifdef _WIN32
#  include <io.h>
#  include <windows.h>
#  define O_BINARY _O_BINARY
#  define ssize_t  int
#else
#  include <unistd.h>
#  define O_BINARY 0
#endif

static void print_proxy_config(int32_t option_count, char *options[]) {
    if (!option_count)
        printf("Proxy configuration\n");

    if (!option_count || strcmp(*options, "auto_discover") == 0) {
        if (!option_count)
            printf("  Auto discover: ");
        bool auto_discover = proxy_config_get_auto_discover();
        printf("%s\n", auto_discover ? "enabled" : "disabled");
    }

    if (!option_count || strcmp(*options, "auto_config_url") == 0) {
        if (!option_count)
            printf("  Auto config url: ");
        char *auto_config_url = proxy_config_get_auto_config_url();
        if (auto_config_url) {
            printf("%s\n", auto_config_url);
            free(auto_config_url);
        } else {
            printf("not set\n");
        }
    }

    if (!option_count || strcmp(*options, "proxy") == 0) {
        if (!option_count)
            printf("  Proxy: ");
        const char *scheme = option_count > 1 ? options[1] : "http";
        char *proxy = proxy_config_get_proxy(scheme);
        if (proxy) {
            printf("%s\n", proxy);
            free(proxy);
        } else {
            printf("not set\n");
        }
    }

    if (!option_count || strcmp(*options, "bypass_list") == 0) {
        if (!option_count)
            printf("  Proxy bypass: ");
        char *bypass_list = proxy_config_get_bypass_list();
        if (bypass_list) {
            printf("%s\n", bypass_list);
            free(bypass_list);
        } else {
            printf("not set\n");
        }
    }
}

static bool resolve_proxies_for_url_async(int argc, char *argv[], bool verbose) {
    bool is_ok = false;

    void **proxy_resolver = (void **)calloc(argc, sizeof(void *));
    if (!proxy_resolver)
        return false;

    for (int32_t i = 0; i < argc; i++) {
        // Create each proxy resolver instance
        proxy_resolver[i] = proxy_resolver_create();
        if (!proxy_resolver[i])
            return false;

        // Start asynchronous request for proxy resolution
        proxy_resolver_get_proxies_for_url(proxy_resolver[i], argv[i]);
    }

    is_ok = true;
    for (int32_t i = 0; i < argc; i++) {
        if (verbose)
            printf("Resolving proxy for %s\n", argv[i]);

        // Wait for proxy to resolve asynchronously
        proxy_resolver_wait(proxy_resolver[i], -1);

        // Get the proxy list for the url
        const char *list = proxy_resolver_get_list(proxy_resolver[i]);
        if (verbose)
            printf("  Proxy: ");
        else if (argc > 1)
            printf("%s=", argv[i]);

        printf("%s\n", list ? list : "direct://");

        int32_t error = proxy_resolver_get_error(proxy_resolver[i]);
        if (error != 0) {
            printf("Unable to resolve proxy (%d)\n", error);
            is_ok = false;
        }

        proxy_resolver_delete(&proxy_resolver[i]);
    }

    return is_ok;
}

#ifdef PROXYRES_EXECUTE
static bool execute_pac_script(const char *script_path, const char *url, bool verbose) {
    bool is_ok = false;
    char *script = NULL;

    if (verbose)
        printf("Executing PAC script %s for %s\n", script_path, url);

    // Open PAC script file
    int fd = open(script_path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Failed to open PAC script %s\n", script_path);
        goto execute_pac_cleanup;
    }

    // Get length of PAC script
    off_t script_len = lseek(fd, 0, SEEK_END);
    if (script_len < 0) {
        printf("Failed to get length of PAC script %s\n", script_path);
        goto execute_pac_cleanup;
    }

    // Allocate memory for PAC script
    script = (char *)calloc(script_len + 1, sizeof(char));
    if (!script) {
        printf("Failed to allocate memory for PAC script\n");
        goto execute_pac_cleanup;
    }

    // Read PAC script from file
    lseek(fd, 0, SEEK_SET);
    ssize_t bytes_read = read(fd, script, script_len);
    if (bytes_read != script_len) {
        printf("Failed to read PAC script %s (%" PRId64 " != %" PRId64 ")\n", script_path, (int64_t)bytes_read,
               (int64_t)script_len);
        goto execute_pac_cleanup;
    }

    script[bytes_read] = 0;

    void *proxy_execute = proxy_execute_create();
    if (proxy_execute) {
        if (proxy_execute_get_proxies_for_url(proxy_execute, script, url)) {
            const char *list = proxy_execute_get_list(proxy_execute);
            if (verbose)
                printf("  Proxy: ");
            printf("%s\n", list ? list : "direct://");
        }
        proxy_execute_delete(&proxy_execute);
    }

    is_ok = true;

execute_pac_cleanup:

    close(fd);
    free(script);

    return is_ok;
}
#endif

static int print_help(void) {
    printf("proxyres [--help] [--verbose] cmd cmd_args\n");
    printf(" commands:\n");
    printf("  config                  - dumps all proxy configuration values\n");
#ifdef PROXYRES_EXECUTE
    printf("  execute [file] [urls..] - executes pac file with script\n");
#endif
    printf("  resolve [url..]         - resolves proxy for urls\n");
    return 1;
}

int main(int argc, char *argv[]) {
    int exit_code = 0;
    bool verbose = false;
    int32_t argi = 1;

    if (argc <= 1)
        return print_help();

    if (strcmp(argv[argi], "--verbose") == 0) {
        verbose = true;
        argi++;
    }

    const char *cmd = argv[argi++];
    if (strcmp(cmd, "--help") == 0) {
        print_help();
    } else if (strcmp(cmd, "config") == 0) {
        proxy_config_global_init();
        print_proxy_config(argc - argi, argv + argi);
        proxy_config_global_cleanup();
    } else if (strcmp(cmd, "execute") == 0) {
#ifndef PROXYRES_EXECUTE
        return print_help();
#else
        if (argc <= 3)
            return print_help();
        proxy_execute_global_init();
        const char *script_path = argv[argi];
        while (argi < argc) {
            if (!execute_pac_script(script_path, argv[argi++], verbose))
                exit_code = 1;
        }
        proxy_execute_global_cleanup();
#endif
    } else if (strcmp(cmd, "resolve") == 0) {
        proxy_resolver_global_init();
        if (!resolve_proxies_for_url_async(argc - argi, argv + argi, verbose))
            exit_code = 1;
        proxy_resolver_global_cleanup();
    }

    return exit_code;
}
