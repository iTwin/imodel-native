#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "log.h"
#include "util.h"

#include "curl/curl.h"

typedef struct script_s {
    char *buffer;
    size_t size;
} script_s;

static size_t fetch_write_script(void *contents, size_t size, size_t nscriptb, void *userp) {
    script_s *script = (script_s *)userp;
    size_t new_size = size * nscriptb;
    size_t total_size = script->size + new_size;

    if (total_size > SCRIPT_MAX) {
        log_error("Script size exceeds maximum (%" PRId64 ")", (int64_t)total_size);
        return 0;
    }

    char *ptr = realloc(script->buffer, total_size + 1);
    if (!ptr) {
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "script", errno);
        return 0;
    }

    script->buffer = ptr;
    memcpy(script->buffer + script->size, contents, new_size);
    script->size += new_size;
    script->buffer[script->size] = 0;

    return new_size;
}

// Fetch proxy auto configuration using CURL
char *fetch_get(const char *url, int32_t *error) {
    script_s script = {(char *)calloc(1, sizeof(char)), 0};

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        log_error("Unable to initialize curl handle");
        return NULL;
    }

    // Disable proxy when fetching PAC file
    curl_easy_setopt(curl_handle, CURLOPT_PROXY, "");

    // Add Accept header with PAC mime-type
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/x-ns-proxy-autoconfig");
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

    // Setup url to fetch
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // Write to memory buffer callback
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, fetch_write_script);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&script);

    CURLcode res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        free(script.buffer);
        script.buffer = NULL;
    }

    if (error)
        *error = res;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_handle);
    return script.buffer;
}

bool fetch_global_init(void) {
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        log_error("Unable to initialize curl");
        return false;
    }
    return true;
}

bool fetch_global_cleanup(void) {
    curl_global_cleanup();
    return true;
}
