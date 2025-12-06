# proxy_resolver <!-- omit in toc -->

Resolves proxies for a given URL based on the user's proxy configuration.

* Supports manually specified proxies for specific schemes.
* Supports Web Proxy Auto-Discovery Protocol (WPAD) using DHCP and DNS.
* Evaluates any discovered [Proxy Auto-Configuration (PAC)](https://developer.mozilla.org/en-US/docs/Web/HTTP/Proxy_servers_and_tunneling/Proxy_Auto-Configuration_PAC_file) scripts to determine the proxies for the URL.

The proxy resolution process runs asynchronously. Multiple proxies can be returned so each proxy in the list must be attempted. A direct connection should only be attempted if `direct://` was returned.

**Operating System Support**

The following native OS proxy resolution libraries are used if available:

|OS|Library|Info|
|:-|:-|:-|
|Linux|Gnome3/GProxyResolver|Does not return anything other than direct:// on Ubuntu 20.</br>Currently disabled.|
|macOS|CFNetwork|Always returns proxy type of request URL.|
|Windows|WinHTTP|Uses IE proxy configuration.|

When there is no built-in proxy resolution library on the system, we use our own posix-based resolver.

## API <!-- omit in toc -->

- [proxy\_resolver\_get\_proxies\_for\_url](#proxy_resolver_get_proxies_for_url)
- [proxy\_resolver\_get\_list](#proxy_resolver_get_list)
- [proxy\_resolver\_get\_next\_proxy](#proxy_resolver_get_next_proxy)
- [proxy\_resolver\_get\_error](#proxy_resolver_get_error)
- [proxy\_resolver\_wait](#proxy_resolver_wait)
- [proxy\_resolver\_cancel](#proxy_resolver_cancel)
- [proxy\_resolver\_create](#proxy_resolver_create)
- [proxy\_resolver\_delete](#proxy_resolver_delete)
- [proxy\_resolver\_global\_init](#proxy_resolver_global_init)
- [proxy\_resolver\_global\_cleanup](#proxy_resolver_global_cleanup)

### proxy_resolver_get_proxies_for_url

Asynchronously resolves the proxies for a given URL based on the user's proxy configuration.

**Arguments**
|Type|Name|Description|
|:-|:-|:-|
|void *|ctx|Proxy resolver instance.|
|const char *|url|URL to resolve.|

**Return**
|Type|Description|
|:-|:-|
|bool|`true` if resolved, `false` otherwise.|

### proxy_resolver_get_list

Gets the list of proxies that have been resolved. Each proxy in the list is separated by a comma and in the format: `scheme://host:port`.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void *|ctx|Proxy resolver instance.|

**Return**
|Type|Description|
|-|:-|
|char *|Comma-separated list of proxies uris. For a direct connection, `direct://` is used.|

### proxy_resolver_get_next_proxy

Gets the next proxy in the list of proxies. Caller must free the string returned by this function.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void *|ctx|Proxy resolver instance.|

**Return**
|Type|Description|
|-|:-|
|char *|Next proxy url to use when connecting.|

### proxy_resolver_get_error

Error code for proxy resolution process.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void *|ctx|Proxy resolver instance.|

**Return**
|Type|Description|
|-|:-|
|int32_t|Error code. Varies depending on platform and supported features.|

### proxy_resolver_wait

Wait a specified number of milliseconds for proxy resolution to complete.

**Arguments**
|Type|Name|Description|
|:-|:-|:-|
|void *|ctx|Proxy resolver instance.|
|int32_t|timeout_ms|Number of milliseconds to wait for completion.|

**Return**
|Type|Description|
|-|:-|
|bool|`true` if complete, `false` otherwise.|

### proxy_resolver_cancel

Cancel any pending proxy resolution. Not supported on all platforms or with all native resolvers.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void *|ctx|Proxy resolver instance.|

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

### proxy_resolver_create

Create a proxy resolver instance.

**Return**
|Type|Description|
|-|:-|
|void *|Proxy resolver instance or `NULL` upon failure.|

### proxy_resolver_delete

Deletes a proxy resolver instance.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void **|ctx|Proxy resolver instance pointer.|

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

### proxy_resolver_global_init

Initialization function for proxy resolution. Must be called before any `proxy_resolver` instances are created.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

### proxy_resolver_global_cleanup

Uninitialization function for proxy resolution. Must be called after all `proxy_resolver` instances have been deleted.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

## Example <!-- omit in toc -->

```c
void resolve_proxies_for_url(const char *url) {
    void *proxy_resolver = proxy_resolver_create();
    if (!proxy_resolver)
        return;

    printf("Resolving proxy for %s\n", url);

    if (proxy_resolver_get_proxies_for_url(proxy_resolver, url)) {
        // Wait indefinitely for proxy to resolve asynchronously
        proxy_resolver_wait(proxy_resolver, -1);

        // Get the proxy list for the url
        const char *list = proxy_resolver_get_list(proxy_resolver);
        printf("  Proxy: %s\n", list ? list : "direct://");
    }
    proxy_resolver_delete(&proxy_resolver);
}
```