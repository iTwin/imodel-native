#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <windows.h>
#include <initguid.h>
#include <activscp.h>
#include <activdbg.h>
#include <cguid.h>

#include "execute.h"
#include "execute_i.h"
#include "execute_wsh.h"
#include "log.h"
#include "mozilla_js.h"
#include "net_util.h"
#include "util.h"

#include "execute_wsh.h"
#include "execute_wsh_site.h"

typedef struct proxy_execute_wsh_s {
    // Last error
    int32_t error;
    // Proxy list
    char *list;
    // Windows Script Host interfaces
    active_script_site_s active_script_site;
    IActiveScript *active_script;
    IActiveScriptParse *active_script_parse;
} proxy_execute_wsh_s;

#ifdef _WIN64
#  define IActiveScriptParse_InitNew         IActiveScriptParse64_InitNew
#  define IActiveScriptParse_ParseScriptText IActiveScriptParse64_ParseScriptText
#  define IActiveScriptParse_Release         IActiveScriptParse64_Release
#else
#  define IActiveScriptParse_InitNew         IActiveScriptParse32_InitNew
#  define IActiveScriptParse_ParseScriptText IActiveScriptParse32_ParseScriptText
#  define IActiveScriptParse_Release         IActiveScriptParse32_Release
#endif

static bool script_engine_create(proxy_execute_wsh_s *proxy_execute_wsh) {
    proxy_execute_wsh->active_script_site.ref_count = 0;
    proxy_execute_wsh->active_script_site.site.lpVtbl = active_script_site_get_vtbl();

    active_script_site_add_ref(&proxy_execute_wsh->active_script_site.site);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    CLSID lang_clsid;

    HRESULT result = CLSIDFromProgID(L"JavaScript", &lang_clsid);
    if (FAILED(result)) {
        log_error("Failed to find JavaScript CLSID: (%08lx)", result);
        return false;
    }

    result = CoCreateInstance(&lang_clsid, NULL, CLSCTX_INPROC_SERVER, &IID_IActiveScript,
                              (void **)&proxy_execute_wsh->active_script);
    if (FAILED(result)) {
        log_error("Failed to create active script instance (%08lx)", result);
        return false;
    }

    result = IActiveScript_QueryInterface(proxy_execute_wsh->active_script, &IID_IActiveScriptParse,
                                          (void **)&proxy_execute_wsh->active_script_parse);
    if (FAILED(result)) {
        log_error("Failed to create active script parse instance (%08lx)", result);
        return false;
    }

    result = IActiveScript_SetScriptSite(proxy_execute_wsh->active_script, &proxy_execute_wsh->active_script_site.site);
    if (FAILED(result)) {
        log_error("Failed to set script site for active script (%08lx)", result);
        return false;
    }

    result = IActiveScriptParse_InitNew(proxy_execute_wsh->active_script_parse);
    if (FAILED(result)) {
        log_error("Failed to initialize active script parse (%08lx)", result);
        return false;
    }

    result = IActiveScript_AddNamedItem(proxy_execute_wsh->active_script, WSH_SCRIPT_NAME,
                                        SCRIPTITEM_GLOBALMEMBERS | SCRIPTITEM_ISVISIBLE);
    if (FAILED(result)) {
        log_error("Failed to add named item to active script (%08lx)", result);
        return false;
    }

    result = IActiveScript_SetScriptState(proxy_execute_wsh->active_script, SCRIPTSTATE_STARTED);
    if (FAILED(result)) {
        log_error("Failed to set script state to started (%08lx)", result);
        return false;
    }

    return true;
}

static bool script_engine_parse_text(proxy_execute_wsh_s *proxy_execute_wsh, const char *script) {
    EXCEPINFO excep_info = {0};

    wchar_t *script_wchar = utf8_dup_to_wchar(script);
    if (!script_wchar)
        return false;
    BSTR script_bstr = SysAllocString(script_wchar);
    free(script_wchar);
    if (!script_bstr)
        return false;

    HRESULT result = IActiveScriptParse_ParseScriptText(proxy_execute_wsh->active_script_parse, script_bstr,
                                                        WSH_SCRIPT_NAME, NULL, NULL, 0, 0, 0, NULL, &excep_info);
    SysFreeString(script_bstr);

    if (FAILED(result)) {
        log_error("Failed to parse script text (%08lx)", result);
        return false;
    }

    return true;
}

static bool script_engine_find_proxy_for_url(proxy_execute_wsh_s *proxy_execute_wsh, const char *url) {
    IDispatch *dispatch = NULL;
    DISPID disp_id;
    VARIANTARG params_args[2] = {0};

    HRESULT result = IActiveScript_GetScriptDispatch(proxy_execute_wsh->active_script, NULL, &dispatch);
    if (FAILED(result)) {
        log_error("Failed to get script dispatch (%08lx)", result);
        return false;
    }

    // Get the id for the FindProxyForURL function to call
    LPOLESTR find_proxy_for_url = OLESTR("FindProxyForURL");
    result = IDispatch_GetIDsOfNames(dispatch, &IID_NULL, &find_proxy_for_url, 1, LOCALE_NEUTRAL, &disp_id);
    if (FAILED(result)) {
        log_error("Failed to get ID of names (%08lx)", result);
        goto script_engine_execute_cleanup;
    }

    // Create the parameters for the FindProxyForURL function
    DISPPARAMS params = {params_args, NULL, 2, 0};

    // Create VARIANTARG for the url parameter
    wchar_t *url_wchar = utf8_dup_to_wchar(url);
    if (!url_wchar)
        goto script_engine_execute_cleanup;

    // Arguments for COM calls are passed in reverse order
    VariantInit(&params_args[1]);

    params_args[1].vt = VT_BSTR;
    params_args[1].bstrVal = SysAllocString(url_wchar);
    free(url_wchar);

    // Create VARIANTARG for the host parameter
    char *host = get_url_host(url);
    if (!host) {
        log_error("Failed to get host from url");
        goto script_engine_execute_cleanup;
    }

    wchar_t *host_wchar = utf8_dup_to_wchar(host);
    free(host);
    if (!host_wchar) {
        log_error("Failed to convert host to wchar");
        goto script_engine_execute_cleanup;
    }

    VariantInit(&params_args[0]);

    params_args[0].vt = VT_BSTR;
    params_args[0].bstrVal = SysAllocString(host_wchar);

    free(host_wchar);

    // Invoke the FindProxyForURL function
    VARIANT result_ptr;
    result = IDispatch_Invoke(dispatch, disp_id, &IID_NULL, LOCALE_NEUTRAL, DISPATCH_METHOD, &params, &result_ptr, NULL,
                              NULL);
    if (FAILED(result)) {
        log_error("Failed to invoke script (%08lx)", result);
        goto script_engine_execute_cleanup;
    }

    // Return the result of the FindProxyForURL function
    if (result_ptr.vt != VT_BSTR) {
        log_error("Invalid result type returned from script (%d)", result_ptr.vt);
        goto script_engine_execute_cleanup;
    }

    proxy_execute_wsh->list = wchar_dup_to_utf8(result_ptr.bstrVal);
    if (!proxy_execute_wsh->list)
        goto script_engine_execute_cleanup;

script_engine_execute_cleanup:
    if (params_args[0].bstrVal)
        SysFreeString(params_args[0].bstrVal);
    if (params_args[1].bstrVal)
        SysFreeString(params_args[1].bstrVal);

    if (dispatch)
        IDispatch_Release(dispatch);

    return SUCCEEDED(result);
}

static bool script_engine_delete(proxy_execute_wsh_s *proxy_execute_wsh) {
    if (proxy_execute_wsh->active_script) {
        HRESULT result = IActiveScript_Close(proxy_execute_wsh->active_script);
        if (FAILED(result)) {
            log_error("Failed to close active script (%08lx)", result);
            return false;
        }
    }
    if (proxy_execute_wsh->active_script_parse) {
        HRESULT result = IActiveScriptParse_Release(proxy_execute_wsh->active_script_parse);
        if (FAILED(result)) {
            log_error("Failed to release active script parse (%08lx)", result);
            return false;
        }
    }
    if (proxy_execute_wsh->active_script) {
        HRESULT result = IActiveScript_Release(proxy_execute_wsh->active_script);
        if (FAILED(result)) {
            log_error("Failed to release active script (%08lx)", result);
            return false;
        }
    }

    CoUninitialize();
    return true;
}

bool proxy_execute_wsh_get_proxies_for_url(void *ctx, const char *script, const char *url) {
    proxy_execute_wsh_s *proxy_execute_wsh = (proxy_execute_wsh_s *)ctx;
    bool is_ok = false;

    if (!proxy_execute_wsh || !script || !url)
        return false;

    if (!script_engine_create(proxy_execute_wsh))
        return false;

    if (!script_engine_parse_text(proxy_execute_wsh, MOZILLA_PAC_JAVASCRIPT)) {
        log_error("Failed to parse Mozilla PAC JavaScript");
        goto execute_wsh_cleanup;
    }

    if (!script_engine_parse_text(proxy_execute_wsh, script)) {
        log_error("Failed to parse PAC script");
        goto execute_wsh_cleanup;
    }

    if (!script_engine_find_proxy_for_url(proxy_execute_wsh, url)) {
        log_error("Failed to execute script");
        goto execute_wsh_cleanup;
    }

    is_ok = true;

execute_wsh_cleanup:
    script_engine_delete(proxy_execute_wsh);
    return is_ok;
}

const char *proxy_execute_wsh_get_list(void *ctx) {
    proxy_execute_wsh_s *proxy_execute = (proxy_execute_wsh_s *)ctx;
    return proxy_execute->list;
}

int32_t proxy_execute_wsh_get_error(void *ctx) {
    proxy_execute_wsh_s *proxy_execute = (proxy_execute_wsh_s *)ctx;
    return proxy_execute->error;
}

void *proxy_execute_wsh_create(void) {
    proxy_execute_wsh_s *proxy_execute = (proxy_execute_wsh_s *)calloc(1, sizeof(proxy_execute_wsh_s));
    return proxy_execute;
}

bool proxy_execute_wsh_delete(void **ctx) {
    proxy_execute_wsh_s *proxy_execute;
    if (!ctx)
        return false;
    proxy_execute = (proxy_execute_wsh_s *)*ctx;
    if (!proxy_execute)
        return false;
    free(proxy_execute->list);
    free(proxy_execute);
    *ctx = NULL;
    return true;
}

bool proxy_execute_wsh_global_init(void) {
    return true;
}

bool proxy_execute_wsh_global_cleanup(void) {
    return true;
}

proxy_execute_i_s *proxy_execute_wsh_get_interface(void) {
    static proxy_execute_i_s proxy_execute_wsh_i = {
        proxy_execute_wsh_get_proxies_for_url,
        proxy_execute_wsh_get_list,
        proxy_execute_wsh_get_error,
        proxy_execute_wsh_create,
        proxy_execute_wsh_delete,
        proxy_execute_wsh_global_init,
        proxy_execute_wsh_global_cleanup,
    };
    return &proxy_execute_wsh_i;
}