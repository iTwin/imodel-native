cmake_minimum_required(VERSION 3.12)

# Need to use FetchContent_GetProperties for EXCLUDE_FROM_ALL
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

include(CheckIncludeFile)
include(FeatureSummary)

option(PROXYRES_CURL "Enable support for downloading PAC scripts using curl." OFF)
option(PROXYRES_DUKTAPE "Use embedded Duktape JavaScript engine." OFF)
option(PROXYRES_EXECUTE "Enable support for PAC script execution." ON)

option(PROXYRES_USE_CXX "Use the C++ compiler to compile proxyres." OFF)

option(PROXYRES_BUILD_CLI "Build command line utility." ON)
option(PROXYRES_BUILD_TESTS "Build Googletest unit tests project." ON)

option(PROXYRES_CODE_COVERAGE "Build for code coverage." OFF)

if(NOT PROXYRES_EXECUTE)
    if(UNIX AND NOT APPLE)
        message(STATUS "PAC script execution enabled on linux")
        set(PROXYRES_EXECUTE ON)
    endif()
    if(PROXYRES_DUKTAPE)
        message(STATUS "Duktape engine requires PAC script execution")
        set(PROXYRES_EXECUTE ON)
    endif()
endif()

project(proxyres C CXX)

if(CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel")
    set(PROXYRES_USE_CXX ON)
endif()

set(PROXYRES_HDRS
    include/proxyres/config.h
    include/proxyres/log.h
    include/proxyres/proxyres.h
    include/proxyres/resolver.h)
if(PROXYRES_EXECUTE)
    list(APPEND PROXYRES_HDRS
        include/proxyres/execute.h)
endif()

list(APPEND PROXYRES_HDRS
    config_i.h
    event.h
    log.h
    mutex.h
    net_util.h
    resolver_i.h
    threadpool.h
    util.h)
list(APPEND PROXYRES_SRCS
    config.c
    log.c
    net_util.c
    proxyres.c
    resolver.c
    util.c)
if(PROXYRES_EXECUTE)
    list(APPEND PROXYRES_HDRS
        execute_i.h
        fetch.h
        mozilla_js.h
        net_adapter.h
        resolver_posix.h
        wpad_dhcp_posix.h
        wpad_dhcp.h
        wpad_dns.h)
    list(APPEND PROXYRES_SRCS
        execute.c
        net_adapter.c
        resolver_posix.c
        wpad_dhcp_posix.c
        wpad_dhcp.c
        wpad_dns.c)
    if(PROXYRES_DUKTAPE)
        list(APPEND PROXYRES_HDRS
            execute_duktape.h)
        list(APPEND PROXYRES_SRCS
            execute_duktape.c)
    endif()
endif()
if(WIN32)
    list(APPEND PROXYRES_HDRS
        config_win.h
        util_win.h)
    list(APPEND PROXYRES_SRCS
        config_win.c
        event_win.c
        mutex_win.c
        net_adapter_win.c
        util_win.c)
    if(PROXYRES_EXECUTE)
        list(APPEND PROXYRES_HDRS
            wpad_dhcp_win.h)
        list(APPEND PROXYRES_SRCS
            wpad_dhcp_win.c)
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
        if(NOT PROXYRES_DUKTAPE)
            list(APPEND PROXYRES_HDRS
                resolver_winrt.h)
            list(APPEND PROXYRES_C_SRCS
                resolver_winrt.c)
        endif()
    else()
        if(NOT PROXYRES_DUKTAPE)
            list(APPEND PROXYRES_HDRS
                resolver_win8.h
                resolver_winxp.h)
            list(APPEND PROXYRES_SRCS
                resolver_win8.c
                resolver_winxp.c)
            if(PROXYRES_EXECUTE)
                list(APPEND PROXYRES_HDRS
                    execute_wsh_dispatch.h
                    execute_wsh_site.h
                    execute_wsh.h)
                list(APPEND PROXYRES_C_SRCS
                    execute_wsh.c)
            endif()
        endif()
    endif()

    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION GREATER_EQUAL 6.1)
        list(APPEND PROXYRES_SRCS
            threadpool_winvista.c)
    else()
        list(APPEND PROXYRES_SRCS
            threadpool_winxp.c)
    endif()
elseif(APPLE)
    list(APPEND PROXYRES_HDRS
        config_mac.h)
    list(APPEND PROXYRES_SRCS
        config_mac.c
        event_pthread.c
        mutex_pthread.c
        threadpool_pthread.c)
    if(PROXYRES_EXECUTE)
        list(APPEND PROXYRES_HDRS
            wpad_dhcp_mac.h)
        list(APPEND PROXYRES_SRCS
            net_adapter_mac.c
            wpad_dhcp_mac.c)
        if(NOT PROXYRES_DUKTAPE)
            list(APPEND PROXYRES_HDRS
                execute_jscore.h)
            list(APPEND PROXYRES_SRCS
                execute_jscore.c)
        endif()
    endif()
    if(NOT PROXYRES_DUKTAPE)
        list(APPEND PROXYRES_HDRS
            resolver_mac.h)
        list(APPEND PROXYRES_SRCS
            resolver_mac.c)
    endif()
elseif(UNIX)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GConf gconf-2.0)
        if (GConf_FOUND)
            list(APPEND PROXYRES_HDRS config_gnome2.h)
            list(APPEND PROXYRES_SRCS config_gnome2.c)
        endif()
    endif()

    list(APPEND PROXYRES_HDRS
        config_env.h
        config_gnome3.h
        config_kde.h
        util_linux.h)
    list(APPEND PROXYRES_SRCS
        config_env.c
        config_gnome3.c
        config_kde.c
        event_pthread.c
        mutex_pthread.c
        net_adapter_linux.c
        threadpool_pthread.c
        util_linux.c)
    if(NOT PROXYRES_DUKTAPE)
        list(APPEND PROXYRES_HDRS
            execute_jsc.h
            resolver_gnome3.h)
        list(APPEND PROXYRES_SRCS
            execute_jsc.c
            resolver_gnome3.c)
    endif()
endif()

if(WIN32)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
        COBJMACROS
        WIN32_LEAN_AND_MEAN)
elseif(APPLE)
elseif(UNIX)
    add_compile_definitions(_GNU_SOURCE)
endif()

add_compile_definitions($<IF:$<CONFIG:Debug>,_DEBUG,_NDEBUG>)

if(PROXYRES_CODE_COVERAGE AND NOT MSVC)
    include(CheckCCompilerFlag)

    # Check for -coverage flag support for Clang/GCC
    if(CMAKE_VERSION VERSION_LESS 3.14)
        set(CMAKE_REQUIRED_LIBRARIES -lgcov)
    else()
        set(CMAKE_REQUIRED_LINK_OPTIONS -coverage)
    endif()
    check_c_compiler_flag(-coverage HAVE_COVERAGE)
    set(CMAKE_REQUIRED_LIBRARIES)
    set(CMAKE_REQUIRED_LINK_OPTIONS)

    if(HAVE_COVERAGE)
        add_compile_options(-coverage)
        add_link_options(-coverage)

        if(CMAKE_C_COMPILER_ID MATCHES "GNU")
            link_libraries(gcov)
        endif()

        # Disable optimization for code coverage. Prepend the optimization flags to
        # override any default flags set by CMake for the configuration type on Windows.
        set(CMAKE_C_FLAGS "-O0 ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-O0 ${CMAKE_CXX_FLAGS}")

        message(STATUS "Code coverage enabled using: -coverage")
    else()
        message(WARNING "Code coverage not supported")
    endif()
endif()

if(PROXYRES_USE_CXX)
    set_source_files_properties(${PROXYRES_SRCS} PROPERTIES LANGUAGE CXX)
    if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang")
        set(CMAKE_CXX_FLAGS "-Wno-deprecated ${CMAKE_CXX_FLAGS}")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Intel" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        set(CMAKE_CXX_FLAGS "-Kc++ ${CMAKE_CXX_FLAGS}")
    endif()
endif()

add_library(proxyres ${PROXYRES_HDRS} ${PROXYRES_SRCS} ${PROXYRES_C_SRCS})
set_property(TARGET proxyres PROPERTY C_STANDARD 11 CXX_STANDARD 11)
target_include_directories(proxyres PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/proxyres)
target_include_directories(proxyres PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(NOT WIN32)
    check_include_file("net/if_arp.h" HAVE_NET_IF_ARP_H)
    if(HAVE_NET_IF_ARP_H)
        target_compile_definitions(proxyres PRIVATE HAVE_NET_IF_ARP_H)
    endif()
endif()

if(PROXYRES_CURL AND (PROXYRES_EXECUTE OR PROXYRES_BUILD_CLI))
    if(NOT TARGET CURL::libcurl)
        include(FetchContent)

        # Disable compression and SSL support to avoid pulling in zlib and OpenSSL
        set(CURL_ZLIB OFF CACHE BOOL "Build curl with ZLIB support" FORCE)
        set(CURL_USE_LIBSSH2 OFF CACHE BOOL "Use libSSH2" FORCE)

        # Disable tests
        set(BUILD_CURL_EXE OFF CACHE BOOL "Build executable" FORCE)
        set(CURL_DISABLE_TESTS ON CACHE BOOL "Disable tests" FORCE)
        set(CURL_ENABLE_EXPORT_TARGET OFF CACHE BOOL "to enable cmake export target" FORCE)

        # Disable unnecessary protocols
        set(CURL_DISABLE_HSTS ON CACHE BOOL "to disable HSTS support" FORCE)
        set(CURL_DISABLE_FTP ON CACHE BOOL "Disable FTP" FORCE)
        set(CURL_DISABLE_TELNET ON CACHE BOOL "Disable TELNET" FORCE)
        set(CURL_DISABLE_LDAP ON CACHE BOOL "Disable LDAP" FORCE)
        set(CURL_DISABLE_LDAPS ON CACHE BOOL "Disable LDAPS" FORCE)
        set(CURL_DISABLE_DICT ON CACHE BOOL "Disable DICT" FORCE)
        set(CURL_DISABLE_TFTP ON CACHE BOOL "Disable TFTP" FORCE)
        set(CURL_DISABLE_GOPHER ON CACHE BOOL "Disable GOPHER" FORCE)
        set(CURL_DISABLE_POP3 ON CACHE BOOL "Disable POP3" FORCE)
        set(CURL_DISABLE_IMAP ON CACHE BOOL "Disable IMAP" FORCE)
        set(CURL_DISABLE_SMB ON CACHE BOOL "Disable SMB" FORCE)
        set(CURL_DISABLE_SMTP ON CACHE BOOL "Disable SMTP" FORCE)
        set(CURL_DISABLE_RTSP ON CACHE BOOL "Disable RTSP" FORCE)
        set(CURL_DISABLE_MQTT ON CACHE BOOL "Disable MQTT" FORCE)
        set(CURL_DISABLE_ALTSVC ON CACHE BOOL "Disable alt-svc support" FORCE)
        set(CURL_DISABLE_GETOPTIONS ON CACHE BOOL "Disables curl_easy_options API" FORCE)
        set(CURL_DISABLE_MIME ON CACHE BOOL "Disables MIME support" FORCE)
        set(CURL_DISABLE_NETRC ON CACHE BOOL "Disables netrc parser" FORCE)
        set(CURL_DISABLE_PROGRESS_METER ON CACHE BOOL "Disables built-in progress meter" FORCE)

        # Allow specifying alternative curl repository
        if(NOT DEFINED CURL_REPOSITORY)
            set(CURL_REPOSITORY https://github.com/curl/curl.git)
        endif()

        # Fetch curl source code from official repository
        FetchContent_Declare(curl
            GIT_REPOSITORY ${CURL_REPOSITORY})

        FetchContent_GetProperties(curl)
        if(NOT curl_POPULATED)
            FetchContent_Populate(curl)
            add_subdirectory(${curl_SOURCE_DIR} ${curl_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

        if(NOT TARGET CURL::libcurl)
            add_library(CURL::libcurl ALIAS libcurl)
        endif()
    endif()
endif()

if(PROXYRES_EXECUTE)
    target_compile_definitions(proxyres PUBLIC PROXYRES_EXECUTE)

    if(TARGET CURL::libcurl)
        target_compile_definitions(proxyres PUBLIC HAVE_CURL)
        target_sources(proxyres PRIVATE fetch_curl.c)
        target_link_libraries(proxyres CURL::libcurl)
    else()
        target_sources(proxyres PRIVATE fetch_posix.c)
    endif()
endif()

if(PROXYRES_DUKTAPE)
    include(FetchContent)

    # Only Duktape releases have single-file source distributables
    if(NOT DEFINED DUKTAPE_SOURCE_URL)
        set(DUKTAPE_SOURCE_URL https://github.com/svaarala/duktape/releases/download/v2.7.0/duktape-2.7.0.tar.xz)
    endif()

    FetchContent_Declare(duktape URL ${DUKTAPE_SOURCE_URL} DOWNLOAD_EXTRACT_TIMESTAMP)
    FetchContent_MakeAvailable(duktape)

    add_library(duktape STATIC
        ${duktape_SOURCE_DIR}/src/duktape.c
        ${duktape_SOURCE_DIR}/src/duktape.h
        ${duktape_SOURCE_DIR}/src/duk_config.h)
    target_include_directories(duktape PUBLIC ${duktape_SOURCE_DIR}/src)
    if(UNIX)
        target_link_libraries(duktape m)
    endif()

    target_include_directories(proxyres PRIVATE ${duktape_SOURCE_DIR}/src)
    target_compile_definitions(proxyres PRIVATE HAVE_DUKTAPE)
    target_link_libraries(proxyres duktape)
    target_sources(proxyres PRIVATE
        execute_duktape.h
        execute_duktape.c)
endif()

if(WIN32)
    target_link_libraries(proxyres dhcpcsvc.lib iphlpapi.lib wininet winhttp ws2_32)
elseif(APPLE)
    find_library(CFNETWORK_LIBRARY CFNetwork)
    target_link_libraries(proxyres ${CFNETWORK_LIBRARY})

    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(proxyres ${COREFOUNDATION_LIBRARY})

    find_library(SYSTEMCONFIGURATION_LIBRARY SystemConfiguration)
    target_link_libraries(proxyres ${SYSTEMCONFIGURATION_LIBRARY})

    find_package(Threads REQUIRED)
    target_compile_definitions(proxyres PRIVATE HAVE_PTHREADS)
    target_link_libraries(proxyres ${CMAKE_THREAD_LIBS_INIT})

    set_target_properties(proxyres PROPERTIES LINK_FLAGS -Wl,-F/Library/Frameworks)

    target_compile_definitions(proxyres PRIVATE HAVE_JSCORE)
elseif(UNIX)
    pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
    target_link_libraries(proxyres PkgConfig::deps)

    if(GConf_FOUND)
        # Don't link libraries at compile time since we dynamically load them at runtime
        target_include_directories(proxyres PRIVATE ${GConf_INCLUDE_DIRS})
        target_compile_definitions(proxyres PRIVATE HAVE_GCONF)
    endif()

    if(NOT PROXYRES_DUKTAPE)
        pkg_search_module(JSCoreGTK javascriptcoregtk-6.0 javascriptcoregtk-4.1 javascriptcoregtk-4.0)
        if(JSCoreGTK_FOUND)
            # Don't link libraries at compile time since we dynamically load them at runtime
            target_include_directories(proxyres PRIVATE ${JSCoreGTK_INCLUDE_DIRS})
            target_compile_definitions(proxyres PRIVATE HAVE_JSC)

            pkg_check_modules(GObject REQUIRED gobject-2.0)
            if(GObject_FOUND)
                target_include_directories(proxyres PRIVATE ${GObject_INCLUDE_DIRS})
                target_link_libraries(proxyres ${GObject_LIBRARIES})
            endif()
        endif()
    endif()

    find_package(Threads REQUIRED)
    target_compile_definitions(proxyres PRIVATE HAVE_PTHREADS)
    target_link_libraries(proxyres ${CMAKE_THREAD_LIBS_INIT})

    target_link_libraries(proxyres ${CMAKE_DL_LIBS})
endif()

if(PROXYRES_BUILD_CLI OR PROXYRES_BUILD_TESTS)
    # Should be enabled in source root CMakeLists.txt
    enable_testing()

    add_subdirectory(test)
endif()

add_feature_info(PROXYRES_CURL PROXYRES_CURL "Enable support for downloading PAC scripts using curl.")
add_feature_info(PROXYRES_DUKTAPE PROXYRES_DUKTAPE "Use embedded Duktape JavaScript engine.")
add_feature_info(PROXYRES_EXECUTE PROXYRES_EXECUTE "Enable support for PAC script execution.")

add_feature_info(PROXYRES_USE_CXX PROXYRES_USE_CXX "Use the C++ compiler to compile proxyres.")

add_feature_info(PROXYRES_BUILD_CLI PROXYRES_BUILD_CLI "Build command line utility.")
add_feature_info(PROXYRES_BUILD_TESTS PROXYRES_BUILD_TESTS "Build Googletest unit tests project.")

feature_summary(WHAT ALL)
