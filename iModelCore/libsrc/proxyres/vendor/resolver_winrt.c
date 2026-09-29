#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <wchar.h>

#include <windows.h>
#include <roapi.h>

#include <windows.networking.connectivity.h>

#include "config.h"
#include "event.h"
#include "log.h"
#include "resolver.h"
#include "resolver_i.h"
#include "resolver_winrt.h"
#include "resolver_winxp.h"
#include "util.h"
#include "util_win.h"

#if defined(__cplusplus)
#  define CIID(iid)   (iid)
#  define CGUID(guid) (guid)
#else
#  define CIID(iid)   (&iid)
#  define CGUID(guid) (&guid)
#endif

// There is a bug in the Windows SDK and no WinRT IIDs are defined
const IID IID_INetworkInformationStatics = {
    0x5074F851, 0x950D, 0x4165, {0x9C, 0x15, 0x36, 0x56, 0x19, 0x48, 0x1E, 0xEA}};
const IID IID_IUriRuntimeClassFactory = {0x44A9796F, 0x723E, 0x4FDF, {0xA2, 0x18, 0x03, 0x3E, 0x75, 0xB0, 0xC0, 0x84}};
const IID IID_AsyncOperationCompletedHandler_ProxyConfiguration = {
    0x035B2567, 0xEFB9, 0x5BC3, {0xB6, 0x09, 0xF9, 0xA8, 0xC2, 0x0B, 0x70, 0x01}};

// INetworkInformationStatics
typedef __x_ABI_CWindows_CNetworking_CConnectivity_CINetworkInformationStatics WinRT_INetworkInformationStatics;
#define WinRT_INetworkInformationStatics_GetProxyConfigurationAsync(this, uri, value) \
    __x_ABI_CWindows_CNetworking_CConnectivity_CINetworkInformationStatics_GetProxyConfigurationAsync(this, uri, value)
#define WinRT_INetworkInformationStatics_Release(this) \
    __x_ABI_CWindows_CNetworking_CConnectivity_CINetworkInformationStatics_Release(this)

// IUriRuntimeClass
typedef __x_ABI_CWindows_CFoundation_CIUriRuntimeClass WinRT_IUriRuntimeClass;
#define WinRT_IUriRuntimeClass_GetAbsoluteUri(this, value) \
    __x_ABI_CWindows_CFoundation_CIUriRuntimeClass_get_AbsoluteUri(this, value)
#define WinRT_IUriRuntimeClass_Release(this) __x_ABI_CWindows_CFoundation_CIUriRuntimeClass_Release(this)

// IUriRuntimeClassFactory
typedef __x_ABI_CWindows_CFoundation_CIUriRuntimeClassFactory WinRT_IUriRuntimeClassFactory;
#define WinRT_IUriRuntimeClassFactory_CreateUri(this, uri, instance) \
    __x_ABI_CWindows_CFoundation_CIUriRuntimeClassFactory_CreateUri(this, uri, instance)
#define WinRT_IUriRuntimeClassFactory_Release(this) __x_ABI_CWindows_CFoundation_CIUriRuntimeClassFactory_Release(this)

// IVectorView<Uri>
typedef __FIVectorView_1_Windows__CFoundation__CUri WinRT_IVectorView_Uri;
#define WinRT_IVectorView_Uri_GetAt(This, Index, Item) \
    __FIVectorView_1_Windows__CFoundation__CUri_GetAt(This, Index, Item)
#define WinRT_IVectorView_Uri_GetSize(This, Size) __FIVectorView_1_Windows__CFoundation__CUri_get_Size(This, Size)
#define WinRT_IVectorView_Uri_Release(This)       __FIVectorView_1_Windows__CFoundation__CUri_Release(This)

// IProxyConfiguration
typedef __x_ABI_CWindows_CNetworking_CConnectivity_CIProxyConfiguration WinRT_IProxyConfiguration;
#define WinRT_IProxyConfiguration_GetCanConnectDirectly(This, Value) \
    __x_ABI_CWindows_CNetworking_CConnectivity_CIProxyConfiguration_get_CanConnectDirectly(This, Value)
#define WinRT_IProxyConfiguration_GetProxyUris(This, Value) \
    __x_ABI_CWindows_CNetworking_CConnectivity_CIProxyConfiguration_get_ProxyUris(This, Value)
#define WinRT_IProxyConfiguration_Release(This) \
    __x_ABI_CWindows_CNetworking_CConnectivity_CIProxyConfiguration_Release(This)

// IAsyncOperationCompletedHandler<ProxyConfiguration>
typedef __FIAsyncOperationCompletedHandler_1_Windows__CNetworking__CConnectivity__CProxyConfiguration
    WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration;
typedef __FIAsyncOperationCompletedHandler_1_Windows__CNetworking__CConnectivity__CProxyConfigurationVtbl
    WinRT_IAsyncOperationCompletedHandler_ProxyConfigurationVtbl;

// IAsyncOperation<ProxyConfiguration>
typedef __FIAsyncOperation_1_Windows__CNetworking__CConnectivity__CProxyConfiguration
    WinRT_IAsyncOperation_ProxyConfiguration;
typedef __FIAsyncOperation_1_Windows__CNetworking__CConnectivity__CProxyConfigurationVtbl
    WinRT_IAsyncOperation_ProxyConfigurationVtbl;
#define WinRT_IAsyncOperation_ProxyConfiguration_PutCompleted(self, handler) \
    __FIAsyncOperation_1_Windows__CNetworking__CConnectivity__CProxyConfiguration_put_Completed(self, handler)
#define WinRT_IAsyncOperation_ProxyConfiguration_GetResults(self, results) \
    __FIAsyncOperation_1_Windows__CNetworking__CConnectivity__CProxyConfiguration_GetResults(self, results)
#define WinRT_IAsyncOperation_ProxyConfiguration_Release(self) \
    __FIAsyncOperation_1_Windows__CNetworking__CConnectivity__CProxyConfiguration_Release(self)

struct async_complete_handler_s;

typedef struct proxy_resolver_winrt_s {
    // Last system error
    int32_t error;
    // Proxy list
    char *list;
    // Complete event
    void *complete;
    // Async complete handler
    struct async_complete_handler_s *complete_handler;
} proxy_resolver_winrt_s;

typedef struct async_complete_handler_s {
    // IAsyncOperationCompletedHandler<ProxyConfiguration> interface
    const WinRT_IAsyncOperationCompletedHandler_ProxyConfigurationVtbl *complete;
    // IAsyncOperation<ProxyConfiguration> instance
    WinRT_IAsyncOperation_ProxyConfiguration *async;
    // Reference count
    LONG ref_count;
    // Proxy resolver pointer
    proxy_resolver_winrt_s *proxy_resolver;
} async_complete_handler_s;

#define cast_from_complete_handler_interface(self) \
    (async_complete_handler_s *)((uint8_t *)self - offsetof(async_complete_handler_s, complete))

ULONG STDMETHODCALLTYPE
async_complete_handler_add_ref(WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration *handler) {
    async_complete_handler_s *self = cast_from_complete_handler_interface(handler);
    return InterlockedIncrement(&self->ref_count);
}

ULONG STDMETHODCALLTYPE
async_complete_handler_release(WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration *handler) {
    async_complete_handler_s *self = cast_from_complete_handler_interface(handler);
    if (InterlockedDecrement(&self->ref_count) == 0) {
        free(self);
        return 0;
    }
    return self->ref_count;
}

HRESULT STDMETHODCALLTYPE async_complete_handler_query_interface(
    WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration *handler, REFIID riid, void **ppv) {
    if (IsEqualGUID(riid, CIID(IID_AsyncOperationCompletedHandler_ProxyConfiguration)) ||
        IsEqualGUID(riid, CIID(IID_IAgileObject)) || IsEqualGUID(riid, CIID(IID_IUnknown))) {
        async_complete_handler_add_ref(handler);
        *ppv = handler;
        return NOERROR;
    }
    *ppv = 0;
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE
async_complete_handler_invoke(WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration *handler,
                              WinRT_IAsyncOperation_ProxyConfiguration *async_info, AsyncStatus status) {
    async_complete_handler_s *self = cast_from_complete_handler_interface(handler);
    proxy_resolver_winrt_s *proxy_resolver = self->proxy_resolver;
    WinRT_IProxyConfiguration *results = NULL;
    WinRT_IVectorView_Uri *uri_list = NULL;

    HRESULT result = S_OK;

    if (!proxy_resolver) {
        result = E_CHANGED_STATE;
        log_warn("Proxy resolver was detached from async operation (0x%lx)", result);
        goto winrt_async_done;
    }

    result = WinRT_IAsyncOperation_ProxyConfiguration_GetResults(self->async, &results);
    if (FAILED(result)) {
        log_error("Failed to get results from async operation (0x%lx)", result);
        goto winrt_async_done;
    }

    boolean can_directly_connect = false;
    WinRT_IProxyConfiguration_GetCanConnectDirectly(results, &can_directly_connect);
    if (can_directly_connect) {
        proxy_resolver->list = strdup("direct://");
        goto winrt_async_done;
    }

    // Enumerate through proxy list
    result = WinRT_IProxyConfiguration_GetProxyUris(results, &uri_list);
    if (FAILED(result)) {
        log_error("Failed to get proxy list (0x%lx)", result);
        goto winrt_async_done;
    }

    uint32_t size = 0;
    result = WinRT_IVectorView_Uri_GetSize(uri_list, &size);
    if (size == 0) {
        proxy_resolver->list = strdup("direct://");
        goto winrt_async_done;
    }

    // Allocate string to construct the proxy list into
    int32_t max_list = size * MAX_PROXY_URL;
    proxy_resolver->list = (char *)calloc(max_list, sizeof(char));
    if (!proxy_resolver->list) {
        proxy_resolver->error = ERROR_OUTOFMEMORY;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "proxy list", proxy_resolver->error);
        goto winrt_async_done;
    }

    size_t list_len = 0;

    // Enumerate each proxy returned
    for (uint32_t i = 0; i < size; i++) {
        // Get uri at index in uri list
        WinRT_IUriRuntimeClass *uri = NULL;
        result = WinRT_IVectorView_Uri_GetAt(uri_list, i, &uri);
        if (FAILED(result))
            continue;

        HSTRING url_string = NULL;
        result = WinRT_IUriRuntimeClass_GetAbsoluteUri(uri, &url_string);
        if (SUCCEEDED(result)) {
            const wchar_t *url_wide = WindowsGetStringRawBuffer(url_string, NULL);
            if (url_wide) {
                char *url_utf8 = wchar_dup_to_utf8(url_wide);
                if (url_utf8) {
                    // Append proxy to proxy list
                    strncat(proxy_resolver->list, url_utf8, max_list - list_len - 1);
                    list_len += strlen(url_utf8);
                    free(url_utf8);
                }
                WindowsDeleteString(url_string);
            }
        }

        WinRT_IUriRuntimeClass_Release(uri);

        // Separate each proxy url with a comma
        if (i != size - 1) {
            strncat(proxy_resolver->list, ",", max_list - list_len - 1);
            list_len++;
        }
    }

winrt_async_done:

    proxy_resolver->error = result;

    if (uri_list)
        WinRT_IVectorView_Uri_Release(uri_list);
    if (results)
        WinRT_IProxyConfiguration_Release(results);

    event_set(proxy_resolver->complete);
    return S_OK;
}

static const WinRT_IAsyncOperationCompletedHandler_ProxyConfigurationVtbl async_complete_handler_vtbl = {
    async_complete_handler_query_interface,
    async_complete_handler_add_ref,
    async_complete_handler_release,
    async_complete_handler_invoke,
};

static HRESULT get_activation_factory(const wchar_t *cls, REFIID activatable_cls_id, void **factory) {
    HSTRING cls_string = NULL;

    *factory = NULL;
    HRESULT result = WindowsCreateString(cls, (UINT32)wcslen(cls), &cls_string);
    if (SUCCEEDED(result) && cls_string)
        result = RoGetActivationFactory(cls_string, activatable_cls_id, factory);
    if (cls_string != NULL)
        WindowsDeleteString(cls_string);
    return result;
}

static WinRT_IUriRuntimeClass *create_uri_from_string(const char *url) {
    WinRT_IUriRuntimeClassFactory *uri_factory = NULL;
    WinRT_IUriRuntimeClass *uri = NULL;

    HRESULT result = get_activation_factory(RuntimeClass_Windows_Foundation_Uri, CIID(IID_IUriRuntimeClassFactory),
                                            (void **)&uri_factory);
    if (SUCCEEDED(result)) {
        wchar_t *url_wide = utf8_dup_to_wchar(url);
        if (!url_wide)
            return NULL;

        HSTRING url_string = NULL;
        result = WindowsCreateString(url_wide, (UINT32)wcslen(url_wide), &url_string);
        if (SUCCEEDED(result) && url_string) {
            result = WinRT_IUriRuntimeClassFactory_CreateUri(uri_factory, url_string, &uri);
            WindowsDeleteString(url_string);
        }

        free(url_wide);
        WinRT_IUriRuntimeClassFactory_Release(uri_factory);
    }

    return uri;
}

bool proxy_resolver_winrt_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_winrt_s *proxy_resolver = (proxy_resolver_winrt_s *)ctx;
    WinRT_INetworkInformationStatics *network_info_statics = NULL;
    WinRT_IUriRuntimeClass *uri = NULL;
    bool is_ok = false;

    // Get activation factory instance of NetworkInformationStatics
    HRESULT result = get_activation_factory(RuntimeClass_Windows_Networking_Connectivity_NetworkInformation,
                                            CIID(IID_INetworkInformationStatics), (void **)&network_info_statics);
    if (SUCCEEDED(result)) {
        // Create UriRuntime instance from url string
        uri = create_uri_from_string(url);
        if (!uri) {
            proxy_resolver->error = ERROR_OUTOFMEMORY;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "uri", proxy_resolver->error);
            goto winrt_done;
        }

        // Create instance of IAsyncOperation<ProxyConfiguration> to get async results
        proxy_resolver->complete_handler = (async_complete_handler_s *)malloc(sizeof(async_complete_handler_s));
        if (!proxy_resolver->complete_handler) {
            proxy_resolver->error = ERROR_OUTOFMEMORY;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "async op", proxy_resolver->error);
            goto winrt_done;
        }

        proxy_resolver->complete_handler->complete = &async_complete_handler_vtbl;
        proxy_resolver->complete_handler->ref_count = 1;
        proxy_resolver->complete_handler->proxy_resolver = proxy_resolver;

        // Create proxy configuration async operation
        result = WinRT_INetworkInformationStatics_GetProxyConfigurationAsync(network_info_statics, uri,
                                                                             &proxy_resolver->complete_handler->async);
        if (SUCCEEDED(result)) {
            result = WinRT_IAsyncOperation_ProxyConfiguration_PutCompleted(
                proxy_resolver->complete_handler->async,
                (WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration *)proxy_resolver->complete_handler);
        }
    }

    // Wait for asynchronous proxy configuration operation to complete
    is_ok = true;
    goto winrt_cleanup;

winrt_done:

    is_ok = proxy_resolver->list != NULL || proxy_resolver->error == 0;
    proxy_resolver->error = result;
    event_set(proxy_resolver->complete);

winrt_cleanup:

    if (uri)
        WinRT_IUriRuntimeClass_Release(uri);
    if (network_info_statics)
        WinRT_INetworkInformationStatics_Release(network_info_statics);

    return is_ok;
}

const char *proxy_resolver_winrt_get_list(void *ctx) {
    proxy_resolver_winrt_s *proxy_resolver = (proxy_resolver_winrt_s *)ctx;
    if (!proxy_resolver)
        return NULL;
    return proxy_resolver->list;
}

int32_t proxy_resolver_winrt_get_error(void *ctx) {
    proxy_resolver_winrt_s *proxy_resolver = (proxy_resolver_winrt_s *)ctx;
    return proxy_resolver->error;
}

bool proxy_resolver_winrt_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_winrt_s *proxy_resolver = (proxy_resolver_winrt_s *)ctx;
    if (!proxy_resolver)
        return false;
    return event_wait(proxy_resolver->complete, timeout_ms);
}

bool proxy_resolver_winrt_cancel(void *ctx) {
    return false;
}

void *proxy_resolver_winrt_create(void) {
    proxy_resolver_winrt_s *proxy_resolver = (proxy_resolver_winrt_s *)calloc(1, sizeof(proxy_resolver_winrt_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->complete = event_create();
    if (!proxy_resolver->complete) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_winrt_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_resolver_winrt_s *proxy_resolver = (proxy_resolver_winrt_s *)*ctx;
    if (!proxy_resolver)
        return false;
    proxy_resolver_winrt_cancel(ctx);
    if (proxy_resolver->complete_handler) {
        proxy_resolver->complete_handler->proxy_resolver = NULL;
        if (proxy_resolver->complete_handler->async) {
            async_complete_handler_release(
                (WinRT_IAsyncOperationCompletedHandler_ProxyConfiguration *)proxy_resolver->complete_handler);
        }
    }
    event_delete(&proxy_resolver->complete);
    free(proxy_resolver->list);
    free(proxy_resolver);
    return true;
}

bool proxy_resolver_winrt_global_init(void) {
    return true;
}

bool proxy_resolver_winrt_global_cleanup(void) {
    return true;
}

const proxy_resolver_i_s *proxy_resolver_winrt_get_interface(void) {
    static const proxy_resolver_i_s proxy_resolver_winrt_i = {
        proxy_resolver_winrt_get_proxies_for_url,
        proxy_resolver_winrt_get_list,
        proxy_resolver_winrt_get_error,
        proxy_resolver_winrt_wait,
        proxy_resolver_winrt_cancel,
        proxy_resolver_winrt_create,
        proxy_resolver_winrt_delete,
        true,   // get_proxies_for_url is handled asynchronously
        false,  // get_proxies_for_url does not take into account system config
        proxy_resolver_winrt_global_init,
        proxy_resolver_winrt_global_cleanup};
    return &proxy_resolver_winrt_i;
}
