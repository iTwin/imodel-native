# proxy_config <!-- omit in toc -->

Read the user's proxy configuration.

## API <!-- omit in toc -->

- [proxy\_config\_get\_auto\_discover](#proxy_config_get_auto_discover)
- [proxy\_config\_get\_auto\_config\_url](#proxy_config_get_auto_config_url)
- [proxy\_config\_get\_proxy](#proxy_config_get_proxy)
- [proxy\_config\_get\_bypass\_list](#proxy_config_get_bypass_list)
- [proxy\_config\_set\_auto\_config\_url\_override](#proxy_config_set_auto_config_url_override)
- [proxy\_config\_set\_proxy\_override](#proxy_config_set_proxy_override)
- [proxy\_config\_set\_bypass\_list\_override](#proxy_config_set_bypass_list_override)
- [proxy\_config\_global\_init](#proxy_config_global_init)
- [proxy\_config\_global\_cleanup](#proxy_config_global_cleanup)

### proxy_config_get_auto_discover

Read whether WPAD is enabled on the user's system.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if enabled, `false` otherwise|

**Example**
```c
printf("Auto-discover: %d\n", proxy_config_get_auto_discover());
```

### proxy_config_get_auto_config_url

Read the proxy auto config (PAC) url configured on the user's system.

**Return**
|Type|Description|
|-|:-|
|char *|PAC url, or `NULL` if not configured.<br>Returned string must be released with `free`.|

**Example**
```c
char *auto_config_url = proxy_config_get_auto_config_url();
printf("PAC url: %s\n", auto_config_url ? auto_config_url : "not set");
free(auto_config_url);
```

### proxy_config_get_proxy

Read the proxy configured on the user's system for a given URL scheme. Returned values are in the format of `host:port` and do not include the scheme.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|const char *|scheme|Scheme for proxy.|

**Return**
|Type|Description|
|-|:-|
|char *|Proxy url, or `NULL` if not configured.<br>Returned string must be released with `free`.|

**Example**
```c
char *http_proxy = proxy_config_get_proxy("http");
printf("HTTP proxy: %s\n", http_proxy ? http_proxy : "not set");
free(http_proxy);
```

### proxy_config_get_bypass_list

Read the proxy bypass list configured on the user's system. The list is comma-separated and each expression in the list can be evaluated to determine whether or not proxy evaluation should happen for a particular URL.

**Return**
|Type|Description|
|-|:-|
|char *|Comma-separated bypass list, or `NULL` if not configured.<br>Returned string must be released with `free`.|

**Example**
```c
char *bypass_list = proxy_config_get_bypass_list("http");
printf("Proxy bypass list: %s\n", bypass_list ? bypass_list : "");
free(bypass_list);
```

### proxy_config_set_auto_config_url_override

Override the user's configured proxy auto configuration (PAC) url.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|const char *|auto_config_url|PAC file url|

**Example**
```c
proxy_config_set_auto_config_url_override("http://127.0.0.1:8080/pac.js");
```

### proxy_config_set_proxy_override

Override the user's configured proxy.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|const char *|proxy|Proxy hostname and port|

**Example**
```c
proxy_config_set_auto_config_url_override("127.0.0.1:8080");
```

### proxy_config_set_bypass_list_override

Override the user's configured proxy bypass list.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|const char *|bypass_list|Comma-separated list of proxy bypass expressions.|

**Example**
```c
proxy_config_set_bypass_list_override("complex.com,welldone.com");
```

### proxy_config_global_init

Initialize function for reading user's proxy configuration. Must be called before running any other `proxy_config` function.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

**Example**
```c
proxy_config_global_init();
```

### proxy_config_global_cleanup

Uninitialize function for reading user's proxy configuration. Must be called after all calls to `proxy_config` are finished.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

**Example**
```c
proxy_config_global_cleanup();
```

## Example <!-- omit in toc -->

```c
void print_proxy_config(void) {
    printf("Proxy configuration\n");

    proxy_config_global_init();

    printf("  Auto discover: %s\n",
        proxy_config_get_auto_discover() ? "enabled" : "disabled");

    char *auto_config_url = proxy_config_get_auto_config_url();
    printf("  Auto config url: %s\n", auto_config_url ? auto_config_url : "not set");
    free(auto_config-url);

    char *http_proxy = proxy_config_get_proxy("http");
    printf("  HTTP Proxy: %s\n", http_proxy ? http_proxy : "not set");
    free(http_proxy);

    char *https_proxy = proxy_config_get_proxy("https");
    printf("  HTTPS Proxy: %s\n", https_proxy ? https_proxy : "not set");
    free(https_proxy);

    char *bypass_list = proxy_config_get_bypass_list();
    printf("  Proxy bypass: %s\n", bypass_list ? bypass_list : "not set");
    free(bypass_list);

    proxy_config_global_cleanup();
}
```
