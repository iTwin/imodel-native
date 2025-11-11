/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SQLite/sqlite3.h"
#include <curl/curl.h>
#ifdef __APPLE__
#include <string>
#include <mach-o/dyld.h>
#endif // __APPLE__

#ifdef __APPLE__

static std::string s_certsPath;
static std::string s_empty;

static std::string &getCACertPath() {
    if (s_certsPath.empty()) {
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size); // get the size needed
        if (size == 0) {
            return s_empty;
        }
        std::string executablePath;
        executablePath.resize(size);
        if (_NSGetExecutablePath(&executablePath[0], &size) != 0) {
            return s_empty;
        }
        const std::string suffix = "/Assets/cacert.pem";
        size_t lastSlashIndex = executablePath.rfind('/');
        if (lastSlashIndex != std::string::npos) {
            s_certsPath = executablePath.substr(0, lastSlashIndex) + suffix;
        } else {
            // If there aren't any slashes in the executable path, just use the current directory.
            s_certsPath = std::string(".") + suffix;
        }
    }
    return s_certsPath;
}

void besqlite_bcv_set_cacert_path(const std::string& caFilename) {
    s_certsPath = caFilename;
}

#endif // __APPLE__

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_bcv_curl_handle_config(CURL * pCurl, int /*eMethod*/, const char * /*zUri*/) {
    curl_easy_setopt(pCurl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT);
#ifdef __APPLE__
    const std::string& certsPath = getCACertPath();
    if (certsPath.empty()) {
        return SQLITE_ERROR;
    }
    curl_easy_setopt(pCurl, CURLOPT_CAINFO, certsPath.c_str());
#endif // __APPLE__
    return SQLITE_OK;
}

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
