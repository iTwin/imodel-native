if(PROXYRES_BUILD_CLI)
    set(PROXYCLI_SRCS proxycli.c)

    if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
        set(PROXYCLI_ASSETS
            uwp/Square44x44Logo.png
            uwp/Square150x150Logo.png
            uwp/StoreLogo.png
            uwp/Package.appxmanifest)

        set_source_files_properties(${PROXYCLI_ASSETS} PROPERTIES
            VS_DEPLOYMENT_CONTENT 1
            VS_DEPLOYMENT_LOCATION "Assets")
    endif()

    add_executable(proxycli ${PROXYCLI_SRCS} ${PROXYCLI_ASSETS})
    target_link_libraries(proxycli PRIVATE proxyres)

    if(TARGET CURL::libcurl)
        add_executable(curl_proxyres curl_proxyres.c)
        target_link_libraries(curl_proxyres PRIVATE proxyres CURL::libcurl)

        add_test(NAME curl_proxyres-help
            COMMAND curl_proxyres --help)
        add_test(NAME curl_proxyres-google
            COMMAND curl_proxyres http://google.com/)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
        return()
    endif()

    add_test(NAME proxycli-help
        COMMAND proxycli --help)
    add_test(NAME proxycli-config
        COMMAND proxycli config)
    add_test(NAME proxycli-resolve-google
        COMMAND proxycli resolve https://simple.com/ https://google.com/)
    if(PROXYRES_EXECUTE OR (UNIX AND NOT APPLE))
        add_test(NAME proxycli-execute-google
            COMMAND proxycli execute ${CMAKE_SOURCE_DIR}/test/pac.js https://simple.com/ https://google.com/)
    endif()
endif()

if(PROXYRES_BUILD_TESTS)
    if(NOT TARGET GTest::GTest)
        find_package(GTest QUIET)
    endif()

    if(NOT TARGET GTest::GTest)
        include(FetchContent)

        # Prevent overriding the parent project's compiler/linker settings for Windows
        set(gtest_force_shared_crt ON CACHE BOOL
            "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)

        # Allow specifying alternative Google test repository
        if(NOT DEFINED GTEST_REPOSITORY)
            set(GTEST_REPOSITORY https://github.com/google/googletest.git)
        endif()
        if(NOT DEFINED GTEST_TAG)
            # Use older version of Google test to support older versions of GCC
            if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL 5.3)
                set(GTEST_TAG release-1.10.0)
            else()
                set(GTEST_TAG v1.16.0)
            endif()
        endif()

        # Fetch Google test source code from official repository
        FetchContent_Declare(googletest
            GIT_REPOSITORY ${GTEST_REPOSITORY}
            GIT_TAG ${GTEST_TAG})

        FetchContent_GetProperties(googletest)
        if(NOT googletest_POPULATED)
            FetchContent_Populate(googletest)
            add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

        add_library(GTest::GTest ALIAS gtest)
        add_library(GTest::Main ALIAS gtest_main)
    endif()

    set(TEST_SRCS
        test_config.cc
        test_main.cc
        test_net_util.cc
        test_net_adapter.cc
        test_threadpool.cc
        test_util.cc)
    if(WIN32)
        list(APPEND TEST_SRCS
            test_util_win.cc)
    elseif(UNIX AND NOT APPLE)
        list(APPEND TEST_SRCS
            test_util_linux.cc)
    endif()
    if(PROXYRES_EXECUTE)
        list(APPEND TEST_SRCS
            test_execute.cc
            test_fetch.cc
            test_wpad.cc)
    endif()

    add_executable(gtest_proxyres ${TEST_SRCS})
    target_link_libraries(gtest_proxyres PRIVATE proxyres GTest::GTest)
    target_include_directories(gtest_proxyres PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include/proxyres)

    add_test(NAME gtest_proxyres COMMAND gtest_proxyres --gtest_filter=*:-wpad*)
    add_test(NAME gtest_proxyres-wpad COMMAND gtest_proxyres --gtest_filter=wpad*)
endif()
