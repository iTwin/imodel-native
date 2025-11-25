#include <stdint.h>
#include <stdbool.h>

#include <duktape.h>

#include "execute.h"
#include "execute_i.h"
#include "execute_duktape.h"
#include "log.h"
#include "mozilla_js.h"
#include "net_util.h"
#include "util.h"

typedef struct proxy_execute_duktape_s {
    // Execute error
    int32_t error;
    // Proxy list
    char *list;
    // Duktape heap
    duk_context *ctx;
} proxy_execute_duktape_s;

static duk_ret_t proxy_execute_duktape_dns_resolve(duk_context *ctx) {
    if (duk_get_top(ctx) != 1 || !duk_is_string(ctx, 0))
        return DUK_RET_TYPE_ERROR;

    const char *host = duk_get_string(ctx, 0);
    if (!host)
        return DUK_RET_ERROR;

    char *address = dns_resolve(host, NULL);
    duk_push_string(ctx, address ? address : "");
    free(address);
    return 1;
}

static duk_ret_t proxy_execute_duktape_dns_resolve_ex(duk_context *ctx) {
    if (duk_get_top(ctx) != 1 || !duk_is_string(ctx, 0))
        return DUK_RET_TYPE_ERROR;

    const char *host = duk_get_string(ctx, 0);
    if (!host)
        return DUK_RET_ERROR;

    char *address = dns_resolve_ex(host, NULL);
    duk_push_string(ctx, address ? address : "");
    free(address);
    return 1;
}

static duk_ret_t proxy_execute_duktape_my_ip_address(duk_context *ctx) {
    char *ip = my_ip_address();
    duk_push_string(ctx, ip ? ip : "");
    free(ip);
    return 1;
}

static duk_ret_t proxy_execute_duktape_my_ip_address_ex(duk_context *ctx) {
    char *ip = my_ip_address_ex();
    duk_push_string(ctx, ip ? ip : "");
    free(ip);
    return 1;
}

bool proxy_execute_duktape_get_proxies_for_url(void *ctx, const char *script, const char *url) {
    proxy_execute_duktape_s *proxy_execute = (proxy_execute_duktape_s *)ctx;
    if (!proxy_execute || !proxy_execute->ctx)
        return false;

    duk_context *duk_ctx = proxy_execute->ctx;

    static struct {
        const char *name;
        duk_c_function callback;
        int nargs;
    } functions[] = {
        {"dnsResolve", proxy_execute_duktape_dns_resolve, 1},
        {"dnsResolveEx", proxy_execute_duktape_dns_resolve_ex, 1},
        {"myIpAddress", proxy_execute_duktape_my_ip_address, 0},
        {"myIpAddressEx", proxy_execute_duktape_my_ip_address_ex, 0},
    };

    // Register native functions with JavaScript engine
    for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
        duk_push_c_function(duk_ctx, functions[i].callback, functions[i].nargs);
        duk_put_global_string(duk_ctx, functions[i].name);
    }

    // Load Mozilla's JavaScript PAC utilities to help process PAC files
    if (duk_peval_string(duk_ctx, MOZILLA_PAC_JAVASCRIPT) != 0) {
        log_error("Failed to parse Mozilla PAC JavaScript");
        duk_pop(duk_ctx);
        return false;
    }

    // Evaluate the PAC script
    if (duk_peval_string(duk_ctx, script) != 0) {
        log_error("Error evaluating PAC script: %s", duk_safe_to_string(duk_ctx, -1));
        duk_pop(duk_ctx);
        return false;
    }
    duk_pop(duk_ctx);

    // Construct the call FindProxyForURL
    duk_push_global_object(duk_ctx);
    duk_get_prop_string(duk_ctx, -1, "FindProxyForURL");
    if (!duk_is_function(duk_ctx, -1)) {
        log_error("FindProxyForURL is not a function");
        duk_pop_2(duk_ctx);
        return false;
    }

    duk_push_string(duk_ctx, url);
    char *host = get_url_host(url);
    duk_push_string(duk_ctx, host ? host : url);
    free(host);

    // Execute the call to FindProxyForURL
    if (duk_pcall(duk_ctx, 2) != 0) {
        log_error("Error calling FindProxyForURL: %s", duk_safe_to_string(duk_ctx, -1));
        duk_pop_2(duk_ctx);
        return false;
    }

    if (duk_is_string(duk_ctx, -1)) {
        const char *list = duk_get_string(duk_ctx, -1);
        proxy_execute->list = list ? strdup(list) : NULL;
    }

    duk_pop_2(duk_ctx);
    return proxy_execute->list != NULL;
}

const char *proxy_execute_duktape_get_list(void *ctx) {
    proxy_execute_duktape_s *proxy_execute = (proxy_execute_duktape_s *)ctx;
    return proxy_execute->list;
}

int32_t proxy_execute_duktape_get_error(void *ctx) {
    proxy_execute_duktape_s *proxy_execute = (proxy_execute_duktape_s *)ctx;
    return proxy_execute->error;
}

void *proxy_execute_duktape_create(void) {
    proxy_execute_duktape_s *proxy_execute = (proxy_execute_duktape_s *)calloc(1, sizeof(proxy_execute_duktape_s));
    if (!proxy_execute)
        return NULL;

    proxy_execute->ctx = duk_create_heap_default();
    if (!proxy_execute->ctx) {
        free(proxy_execute);
        return NULL;
    }

    return proxy_execute;
}

bool proxy_execute_duktape_delete(void **ctx) {
    if (!ctx || !*ctx)
        return false;

    proxy_execute_duktape_s *proxy_execute = (proxy_execute_duktape_s *)*ctx;
    if (proxy_execute->list)
        free(proxy_execute->list);
    if (proxy_execute->ctx)
        duk_destroy_heap(proxy_execute->ctx);

    free(proxy_execute);
    *ctx = NULL;
    return true;
}

bool proxy_execute_duktape_global_init(void) {
    return true;
}

bool proxy_execute_duktape_global_cleanup(void) {
    return true;
}

proxy_execute_i_s *proxy_execute_duktape_get_interface(void) {
    static proxy_execute_i_s proxy_execute_duktape_i = {proxy_execute_duktape_get_proxies_for_url,
                                                        proxy_execute_duktape_get_list,
                                                        proxy_execute_duktape_get_error,
                                                        proxy_execute_duktape_create,
                                                        proxy_execute_duktape_delete,
                                                        proxy_execute_duktape_global_init,
                                                        proxy_execute_duktape_global_cleanup};
    return &proxy_execute_duktape_i;
}
