# proxy_execute <!-- omit in toc -->

Executes a Proxy Auto-Configuration (PAC) script containing the JavasScript function `FindProxyForURL` for a particular URL to determine its proxies. Support varies depending on the operating system and the libraries available.

The script execution process is blocking.

#### Script Engine Support by Operating System

|OS|Engine|Info|
|:-|:-|:-|
|Linux|JavaScriptCoreGTK|Dynamically loaded at run-time.|
|macOS|JavaScriptCore|Dynamically loaded at run-time.|
|Windows|Windows Script Host|Uses IActiveScript COM interfaces.|

## API <!-- omit in toc -->

- [proxy_execute_get_proxies_for_url](#proxy_execute_get_proxies_for_url)
- [proxy_execute_get_list](#proxy_execute_get_list)
- [proxy_execute_get_error](#proxy_execute_get_error)
- [proxy_execute_create](#proxy_execute_create)
- [proxy_execute_delete](#proxy_execute_delete)
- [proxy_execute_global_init](#proxy_execute_global_init)
- [proxy_execute_global_cleanup](#proxy_execute_global_cleanup)

### proxy_execute_get_proxies_for_url

Executes a PAC script for a particular URL.

**Arguments**
|Type|Name|Description|
|:-|:-|:-|
|void *|ctx|Proxy execute instance.|
|const char *|script|PAC JavaScript null-terminated string.|
|const char *|url|URL to evaluate.|

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

### proxy_execute_get_list

Get the list of proxies returned by the call to `FindProxyForURL`.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void *|ctx|Proxy execute instance.|

**Return**
|Type|Description|
|-|:-|
|const char *|List of proxies returned by script.|

### proxy_execute_get_error

Error code for script execution. Value varies depending upon script engine and platform.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void *|ctx|Proxy execute instance.|

**Return**
|Type|Description|
|-|:-|
|int32_t|Error code.|

### proxy_execute_create

Create new PAC script execution instance.

**Return**
|Type|Description|
|-|:-|
|void *|Pointer to new instance or `NULL` upon failure.|

### proxy_execute_delete

Delete a PAC script execution instance.

**Arguments**
|Type|Name|Description|
|-|-|:-|
|void **|ctx|Pointer to instance.|

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

### proxy_execute_global_init

Initialization function for PAC script execution. Must be called before any `proxy_execute` instances are created.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

### proxy_execute_global_cleanup

Uninitialization function for PAC script execution. Must be called after all `proxy_execute` instances have been deleted.

**Return**
|Type|Description|
|-|:-|
|bool|`true` if successful, `false` otherwise.|

## Example

```c
static const char *pac_script = R"(
function FindProxyForURL(url, host) {
  if (host == "simple.com") {
    return "PROXY no-such-proxy:80";
  }
  return "DIRECT";
})";
static const char *url = "https://simple.com/hello-world";

bool execute_simple(void) {
    bool is_ok = false
    void *proxy_execute = proxy_execute_create();
    if (proxy_execute) {
        if (!proxy_execute_get_proxies_for_url(proxy_execute, pac_script, url)) {
            printf("Failed to execute PAC script for url %s (%d)\n", url,
                proxy_execute_get_error(proxy_execute));
        } else {
            const char *list = proxy_execute_get_list(proxy_execute);
            if (list) {
                printf("FindProxyForURL for %s = %s\n", url, list);
                is_ok = true;
            }
            free(list);
        }
        proxy_execute_delete(&proxy_execute);
    }
    return is_ok;
}

```