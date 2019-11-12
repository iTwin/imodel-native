/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Napi/node_api.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

/** This file exists to resolve the references to the Node "napi" layer from our .dlls 
 * (e.g. DgnPlatform and iModelJs.node).
 * See README.md for explanation.
 */

#if defined(_WIN32)
#pragma warning(disable : 4100)
#define ABORT_NO_BACKTRACE() raise(SIGABRT)
#define FORWARD_EXPORT(name) __pragma(comment(linker, "/export:"##name))
#elif defined(__clang__)
#define ABORT_NO_BACKTRACE() abort()
#define FORWARD_EXPORT(name)
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wconstant-conversion"
#endif

#define FORWARD_NAPI_EXPORT(name) FORWARD_EXPORT("napi_"##name)

#if defined(BUILD_FOR_NODE)

#if defined(_WIN32)
#include <windows.h>
#include <delayimp.h>

// On Windows, we delay load node.lib. It declares all of its exports to be from "node.exe".
// We Register a delay-load hook to redirect any node.exe references to be from the loading executable.
// This way the addon will still work even if the host executable is not named node.exe.
static FARPROC delayLoadNotify(unsigned dliNotify, PDelayLoadInfo pdli)
{
    if (dliNotePreLoadLibrary != dliNotify || 0 != _stricmp(pdli->szDll, "node.exe"))
        return nullptr;
    return (FARPROC)::GetModuleHandle(nullptr);
}
decltype(__pfnDliNotifyHook2) __pfnDliNotifyHook2 = delayLoadNotify;
#endif

FORWARD_NAPI_EXPORT("acquire_threadsafe_function")
FORWARD_NAPI_EXPORT("add_env_cleanup_hook")
FORWARD_NAPI_EXPORT("add_finalizer")
FORWARD_NAPI_EXPORT("adjust_external_memory")
FORWARD_NAPI_EXPORT("async_destroy")
FORWARD_NAPI_EXPORT("async_init")
FORWARD_NAPI_EXPORT("call_function")
FORWARD_NAPI_EXPORT("call_threadsafe_function")
FORWARD_NAPI_EXPORT("cancel_async_work")
FORWARD_NAPI_EXPORT("close_callback_scope")
FORWARD_NAPI_EXPORT("close_escapable_handle_scope")
FORWARD_NAPI_EXPORT("close_handle_scope")
FORWARD_NAPI_EXPORT("coerce_to_bool")
FORWARD_NAPI_EXPORT("coerce_to_number")
FORWARD_NAPI_EXPORT("coerce_to_object")
FORWARD_NAPI_EXPORT("coerce_to_string")
FORWARD_NAPI_EXPORT("create_array")
FORWARD_NAPI_EXPORT("create_array_with_length")
FORWARD_NAPI_EXPORT("create_arraybuffer")
FORWARD_NAPI_EXPORT("create_async_work")
FORWARD_NAPI_EXPORT("create_bigint_int64")
FORWARD_NAPI_EXPORT("create_bigint_uint64")
FORWARD_NAPI_EXPORT("create_bigint_words")
FORWARD_NAPI_EXPORT("create_buffer")
FORWARD_NAPI_EXPORT("create_buffer_copy")
FORWARD_NAPI_EXPORT("create_dataview")
FORWARD_NAPI_EXPORT("create_double")
FORWARD_NAPI_EXPORT("create_error")
FORWARD_NAPI_EXPORT("create_external")
FORWARD_NAPI_EXPORT("create_external_arraybuffer")
FORWARD_NAPI_EXPORT("create_external_buffer")
FORWARD_NAPI_EXPORT("create_function")
FORWARD_NAPI_EXPORT("create_int32")
FORWARD_NAPI_EXPORT("create_int64")
FORWARD_NAPI_EXPORT("create_object")
FORWARD_NAPI_EXPORT("create_promise")
FORWARD_NAPI_EXPORT("create_range_error")
FORWARD_NAPI_EXPORT("create_reference")
FORWARD_NAPI_EXPORT("create_string_latin1")
FORWARD_NAPI_EXPORT("create_string_utf16")
FORWARD_NAPI_EXPORT("create_string_utf8")
FORWARD_NAPI_EXPORT("create_symbol")
FORWARD_NAPI_EXPORT("create_threadsafe_function")
FORWARD_NAPI_EXPORT("create_type_error")
FORWARD_NAPI_EXPORT("create_typedarray")
FORWARD_NAPI_EXPORT("create_uint32")
FORWARD_NAPI_EXPORT("define_class")
FORWARD_NAPI_EXPORT("define_properties")
FORWARD_NAPI_EXPORT("delete_async_work")
FORWARD_NAPI_EXPORT("delete_element")
FORWARD_NAPI_EXPORT("delete_property")
FORWARD_NAPI_EXPORT("delete_reference")
FORWARD_NAPI_EXPORT("escape_handle")
FORWARD_NAPI_EXPORT("fatal_error")
FORWARD_NAPI_EXPORT("fatal_exception")
FORWARD_NAPI_EXPORT("get_and_clear_last_exception")
FORWARD_NAPI_EXPORT("get_array_length")
FORWARD_NAPI_EXPORT("get_arraybuffer_info")
FORWARD_NAPI_EXPORT("get_boolean")
FORWARD_NAPI_EXPORT("get_buffer_info")
FORWARD_NAPI_EXPORT("get_cb_info")
FORWARD_NAPI_EXPORT("get_dataview_info")
FORWARD_NAPI_EXPORT("get_element")
FORWARD_NAPI_EXPORT("get_global")
FORWARD_NAPI_EXPORT("get_last_error_info")
FORWARD_NAPI_EXPORT("get_named_property")
FORWARD_NAPI_EXPORT("get_new_target")
FORWARD_NAPI_EXPORT("get_node_version")
FORWARD_NAPI_EXPORT("get_null")
FORWARD_NAPI_EXPORT("get_property")
FORWARD_NAPI_EXPORT("get_property_names")
FORWARD_NAPI_EXPORT("get_prototype")
FORWARD_NAPI_EXPORT("get_reference_value")
FORWARD_NAPI_EXPORT("get_threadsafe_function_context")
FORWARD_NAPI_EXPORT("get_typedarray_info")
FORWARD_NAPI_EXPORT("get_undefined")
FORWARD_NAPI_EXPORT("get_uv_event_loop")
FORWARD_NAPI_EXPORT("get_value_bigint_int64")
FORWARD_NAPI_EXPORT("get_value_bigint_uint64")
FORWARD_NAPI_EXPORT("get_value_bigint_words")
FORWARD_NAPI_EXPORT("get_value_bool")
FORWARD_NAPI_EXPORT("get_value_double")
FORWARD_NAPI_EXPORT("get_value_external")
FORWARD_NAPI_EXPORT("get_value_int32")
FORWARD_NAPI_EXPORT("get_value_int64")
FORWARD_NAPI_EXPORT("get_value_string_latin1")
FORWARD_NAPI_EXPORT("get_value_string_utf16")
FORWARD_NAPI_EXPORT("get_value_string_utf8")
FORWARD_NAPI_EXPORT("get_value_uint32")
FORWARD_NAPI_EXPORT("get_version")
FORWARD_NAPI_EXPORT("has_element")
FORWARD_NAPI_EXPORT("has_named_property")
FORWARD_NAPI_EXPORT("has_own_property")
FORWARD_NAPI_EXPORT("has_property")
FORWARD_NAPI_EXPORT("instanceof")
FORWARD_NAPI_EXPORT("is_array")
FORWARD_NAPI_EXPORT("is_arraybuffer")
FORWARD_NAPI_EXPORT("is_buffer")
FORWARD_NAPI_EXPORT("is_dataview")
FORWARD_NAPI_EXPORT("is_error")
FORWARD_NAPI_EXPORT("is_exception_pending")
FORWARD_NAPI_EXPORT("is_promise")
FORWARD_NAPI_EXPORT("is_typedarray")
FORWARD_NAPI_EXPORT("make_callback")
FORWARD_NAPI_EXPORT("module_register")
FORWARD_NAPI_EXPORT("new_instance")
FORWARD_NAPI_EXPORT("open_callback_scope")
FORWARD_NAPI_EXPORT("open_escapable_handle_scope")
FORWARD_NAPI_EXPORT("open_handle_scope")
FORWARD_NAPI_EXPORT("queue_async_work")
FORWARD_NAPI_EXPORT("ref_threadsafe_function")
FORWARD_NAPI_EXPORT("reference_ref")
FORWARD_NAPI_EXPORT("reference_unref")
FORWARD_NAPI_EXPORT("reject_deferred")
FORWARD_NAPI_EXPORT("release_threadsafe_function")
FORWARD_NAPI_EXPORT("remove_env_cleanup_hook")
FORWARD_NAPI_EXPORT("remove_wrap")
FORWARD_NAPI_EXPORT("resolve_deferred")
FORWARD_NAPI_EXPORT("run_script")
FORWARD_NAPI_EXPORT("set_element")
FORWARD_NAPI_EXPORT("set_named_property")
FORWARD_NAPI_EXPORT("set_property")
FORWARD_NAPI_EXPORT("strict_equals")
FORWARD_NAPI_EXPORT("throw")
FORWARD_NAPI_EXPORT("throw_error")
FORWARD_NAPI_EXPORT("throw_range_error")
FORWARD_NAPI_EXPORT("throw_type_error")
FORWARD_NAPI_EXPORT("typeof")
FORWARD_NAPI_EXPORT("unref_threadsafe_function")
FORWARD_NAPI_EXPORT("unwrap")
FORWARD_NAPI_EXPORT("wrap")

#else

EXTERN_C_START

NAPI_EXTERN void napi_module_register(napi_module *) {}

NAPI_EXTERN napi_status
napi_get_last_error_info(napi_env,
                         const napi_extended_error_info **) { return napi_ok; }

NAPI_EXTERN NAPI_NO_RETURN void napi_fatal_error(const char *location,
                                                 size_t location_len,
                                                 const char *message,
                                                 size_t message_len) { ABORT_NO_BACKTRACE(); }

// Getters for defined singletons
NAPI_EXTERN napi_status napi_get_undefined(napi_env, napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_null(napi_env env, napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_global(napi_env env, napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_boolean(napi_env env,
                                         bool value,
                                         napi_value *result) { return napi_ok; }

// Methods to create Primitive types/Objects
NAPI_EXTERN napi_status napi_create_object(napi_env env, napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_array(napi_env env, napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_array_with_length(napi_env env,
                                                      size_t length,
                                                      napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_double(napi_env env,
                                           double value,
                                           napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_int32(napi_env env,
                                          int32_t value,
                                          napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_uint32(napi_env env,
                                           uint32_t value,
                                           napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_int64(napi_env env,
                                          int64_t value,
                                          napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_string_latin1(napi_env env,
                                                  const char *str,
                                                  size_t length,
                                                  napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_string_utf8(napi_env env,
                                                const char *str,
                                                size_t length,
                                                napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_string_utf16(napi_env env,
                                                 const char16_t *str,
                                                 size_t length,
                                                 napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_symbol(napi_env env,
                                           napi_value description,
                                           napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_function(napi_env env,
                                             const char *utf8name,
                                             size_t length,
                                             napi_callback cb,
                                             void *data,
                                             napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_error(napi_env env,
                                          napi_value code,
                                          napi_value msg,
                                          napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_type_error(napi_env env,
                                               napi_value code,
                                               napi_value msg,
                                               napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_range_error(napi_env env,
                                                napi_value code,
                                                napi_value msg,
                                                napi_value *result) { return napi_ok; }

// Methods to get the the native napi_value from Primitive type
NAPI_EXTERN napi_status napi_typeof(napi_env env,
                                    napi_value value,
                                    napi_valuetype *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_value_double(napi_env env,
                                              napi_value value,
                                              double *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_value_int32(napi_env env,
                                             napi_value value,
                                             int32_t *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_value_uint32(napi_env env,
                                              napi_value value,
                                              uint32_t *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_value_int64(napi_env env,
                                             napi_value value,
                                             int64_t *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_value_bool(napi_env env,
                                            napi_value value,
                                            bool *result) { return napi_ok; }

// Copies LATIN-1 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_latin1(napi_env env,
                                                     napi_value value,
                                                     char *buf,
                                                     size_t bufsize,
                                                     size_t *result) { return napi_ok; }

// Copies UTF-8 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_utf8(napi_env env,
                                                   napi_value value,
                                                   char *buf,
                                                   size_t bufsize,
                                                   size_t *result) { return napi_ok; }

// Copies UTF-16 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_utf16(napi_env env,
                                                    napi_value value,
                                                    char16_t *buf,
                                                    size_t bufsize,
                                                    size_t *result) { return napi_ok; }

// Methods to coerce values
// These APIs may execute user scripts
NAPI_EXTERN napi_status napi_coerce_to_bool(napi_env env,
                                            napi_value value,
                                            napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_coerce_to_number(napi_env env,
                                              napi_value value,
                                              napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_coerce_to_object(napi_env env,
                                              napi_value value,
                                              napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_coerce_to_string(napi_env env,
                                              napi_value value,
                                              napi_value *result) { return napi_ok; }

// Methods to work with Objects
NAPI_EXTERN napi_status napi_get_prototype(napi_env env,
                                           napi_value object,
                                           napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_property_names(napi_env env,
                                                napi_value object,
                                                napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_set_property(napi_env env,
                                          napi_value object,
                                          napi_value key,
                                          napi_value value) { return napi_ok; }
NAPI_EXTERN napi_status napi_has_property(napi_env env,
                                          napi_value object,
                                          napi_value key,
                                          bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_property(napi_env env,
                                          napi_value object,
                                          napi_value key,
                                          napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_delete_property(napi_env env,
                                             napi_value object,
                                             napi_value key,
                                             bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_has_own_property(napi_env env,
                                              napi_value object,
                                              napi_value key,
                                              bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_set_named_property(napi_env env,
                                                napi_value object,
                                                const char *utf8name,
                                                napi_value value) { return napi_ok; }
NAPI_EXTERN napi_status napi_has_named_property(napi_env env,
                                                napi_value object,
                                                const char *utf8name,
                                                bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_named_property(napi_env env,
                                                napi_value object,
                                                const char *utf8name,
                                                napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_set_element(napi_env env,
                                         napi_value object,
                                         uint32_t index,
                                         napi_value value) { return napi_ok; }
NAPI_EXTERN napi_status napi_has_element(napi_env env,
                                         napi_value object,
                                         uint32_t index,
                                         bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_element(napi_env env,
                                         napi_value object,
                                         uint32_t index,
                                         napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_delete_element(napi_env env,
                                            napi_value object,
                                            uint32_t index,
                                            bool *result) { return napi_ok; }
NAPI_EXTERN napi_status
napi_define_properties(napi_env env,
                       napi_value object,
                       size_t property_count,
                       const napi_property_descriptor *properties) { return napi_ok; }

// Methods to work with Arrays
NAPI_EXTERN napi_status napi_is_array(napi_env env,
                                      napi_value value,
                                      bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_array_length(napi_env env,
                                              napi_value value,
                                              uint32_t *result) { return napi_ok; }

// Methods to compare values
NAPI_EXTERN napi_status napi_strict_equals(napi_env env,
                                           napi_value lhs,
                                           napi_value rhs,
                                           bool *result) { return napi_ok; }

// Methods to work with Functions
NAPI_EXTERN napi_status napi_call_function(napi_env env,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value *argv,
                                           napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_new_instance(napi_env env,
                                          napi_value constructor,
                                          size_t argc,
                                          const napi_value *argv,
                                          napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_instanceof(napi_env env,
                                        napi_value object,
                                        napi_value constructor,
                                        bool *result) { return napi_ok; }

// Methods to work with napi_callbacks

// Gets all callback info in a single call. (Ugly, but faster.)
NAPI_EXTERN napi_status napi_get_cb_info(
    napi_env env,              // [in] NAPI environment handle
    napi_callback_info cbinfo, // [in] Opaque callback-info handle
    size_t *argc,              // [in-out] Specifies the size of the provided argv array
                               // and receives the actual count of args.
    napi_value *argv,          // [out] Array of values
    napi_value *this_arg,      // [out] Receives the JS 'this' arg for the call
    void **data)
{
    return napi_ok;
} // [out] Receives the data pointer for the callback.

NAPI_EXTERN napi_status napi_get_new_target(napi_env env,
                                            napi_callback_info cbinfo,
                                            napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status
napi_define_class(napi_env env,
                  const char *utf8name,
                  size_t length,
                  napi_callback constructor,
                  void *data,
                  size_t property_count,
                  const napi_property_descriptor *properties,
                  napi_value *result) { return napi_ok; }

// Methods to work with external data objects
NAPI_EXTERN napi_status napi_wrap(napi_env env,
                                  napi_value js_object,
                                  void *native_object,
                                  napi_finalize finalize_cb,
                                  void *finalize_hint,
                                  napi_ref *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_unwrap(napi_env env,
                                    napi_value js_object,
                                    void **result) { return napi_ok; }
NAPI_EXTERN napi_status napi_remove_wrap(napi_env env,
                                         napi_value js_object,
                                         void **result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_external(napi_env env,
                                             void *data,
                                             napi_finalize finalize_cb,
                                             void *finalize_hint,
                                             napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_value_external(napi_env env,
                                                napi_value value,
                                                void **result) { return napi_ok; }

// Methods to control object lifespan

// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
NAPI_EXTERN napi_status napi_create_reference(napi_env env,
                                              napi_value value,
                                              uint32_t initial_refcount,
                                              napi_ref *result) { return napi_ok; }

// Deletes a reference. The referenced value is released, and may
// be GC'd unless there are other references to it.
NAPI_EXTERN napi_status napi_delete_reference(napi_env env, napi_ref ref) { return napi_ok; }

// Increments the reference count, optionally returning the resulting count.
// After this call the  reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
NAPI_EXTERN napi_status napi_reference_ref(napi_env env,
                                           napi_ref ref,
                                           uint32_t *result) { return napi_ok; }

// Decrements the reference count, optionally returning the resulting count.
// If the result is 0 the reference is now weak and the object may be GC'd
// at any time if there are no other references. Calling this when the
// refcount is already 0 results in an error.
NAPI_EXTERN napi_status napi_reference_unref(napi_env env,
                                             napi_ref ref,
                                             uint32_t *result) { return napi_ok; }

// Attempts to get a referenced value. If the reference is weak,
// the value might no longer be available, in that case the call
// is still successful but the result is NULL.
NAPI_EXTERN napi_status napi_get_reference_value(napi_env env,
                                                 napi_ref ref,
                                                 napi_value *result) { return napi_ok; }

NAPI_EXTERN napi_status napi_open_handle_scope(napi_env env,
                                               napi_handle_scope *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_close_handle_scope(napi_env env,
                                                napi_handle_scope scope) { return napi_ok; }
NAPI_EXTERN napi_status
napi_open_escapable_handle_scope(napi_env env,
                                 napi_escapable_handle_scope *result) { return napi_ok; }
NAPI_EXTERN napi_status
napi_close_escapable_handle_scope(napi_env env,
                                  napi_escapable_handle_scope scope) { return napi_ok; }

NAPI_EXTERN napi_status napi_escape_handle(napi_env env,
                                           napi_escapable_handle_scope scope,
                                           napi_value escapee,
                                           napi_value *result) { return napi_ok; }

// Methods to support error handling
NAPI_EXTERN napi_status napi_throw(napi_env env, napi_value error) { return napi_ok; }
NAPI_EXTERN napi_status napi_throw_error(napi_env env,
                                         const char *code,
                                         const char *msg) { return napi_ok; }
NAPI_EXTERN napi_status napi_throw_type_error(napi_env env,
                                              const char *code,
                                              const char *msg) { return napi_ok; }
NAPI_EXTERN napi_status napi_throw_range_error(napi_env env,
                                               const char *code,
                                               const char *msg) { return napi_ok; }
NAPI_EXTERN napi_status napi_is_error(napi_env env,
                                      napi_value value,
                                      bool *result) { return napi_ok; }

// Methods to support catching exceptions
NAPI_EXTERN napi_status napi_is_exception_pending(napi_env env, bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_and_clear_last_exception(napi_env env,
                                                          napi_value *result) { return napi_ok; }

// Methods to provide node::Buffer functionality with napi types
NAPI_EXTERN napi_status napi_create_buffer(napi_env env,
                                           size_t length,
                                           void **data,
                                           napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_external_buffer(napi_env env,
                                                    size_t length,
                                                    void *data,
                                                    napi_finalize finalize_cb,
                                                    void *finalize_hint,
                                                    napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_buffer_copy(napi_env env,
                                                size_t length,
                                                const void *data,
                                                void **result_data,
                                                napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_is_buffer(napi_env env,
                                       napi_value value,
                                       bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_buffer_info(napi_env env,
                                             napi_value value,
                                             void **data,
                                             size_t *length) { return napi_ok; }

// Methods to work with array buffers and typed arrays
NAPI_EXTERN napi_status napi_is_arraybuffer(napi_env env,
                                            napi_value value,
                                            bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_arraybuffer(napi_env env,
                                                size_t byte_length,
                                                void **data,
                                                napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status
napi_create_external_arraybuffer(napi_env env,
                                 void *external_data,
                                 size_t byte_length,
                                 napi_finalize finalize_cb,
                                 void *finalize_hint,
                                 napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_arraybuffer_info(napi_env env,
                                                  napi_value arraybuffer,
                                                  void **data,
                                                  size_t *byte_length) { return napi_ok; }
NAPI_EXTERN napi_status napi_is_typedarray(napi_env env,
                                           napi_value value,
                                           bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_create_typedarray(napi_env env,
                                               napi_typedarray_type type,
                                               size_t length,
                                               napi_value arraybuffer,
                                               size_t byte_offset,
                                               napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_typedarray_info(napi_env env,
                                                 napi_value typedarray,
                                                 napi_typedarray_type *type,
                                                 size_t *length,
                                                 void **data,
                                                 napi_value *arraybuffer,
                                                 size_t *byte_offset) { return napi_ok; }

NAPI_EXTERN napi_status napi_create_dataview(napi_env env,
                                             size_t length,
                                             napi_value arraybuffer,
                                             size_t byte_offset,
                                             napi_value *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_is_dataview(napi_env env,
                                         napi_value value,
                                         bool *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_get_dataview_info(napi_env env,
                                               napi_value dataview,
                                               size_t *bytelength,
                                               void **data,
                                               napi_value *arraybuffer,
                                               size_t *byte_offset) { return napi_ok; }

// Methods to manage simple async operations
NAPI_EXTERN
napi_status napi_create_async_work(napi_env env,
                                   napi_value async_resource,
                                   napi_value async_resource_name,
                                   napi_async_execute_callback execute,
                                   napi_async_complete_callback complete,
                                   void *data,
                                   napi_async_work *result) { return napi_ok; }
NAPI_EXTERN napi_status napi_delete_async_work(napi_env env,
                                               napi_async_work work) { return napi_ok; }
NAPI_EXTERN napi_status napi_queue_async_work(napi_env env,
                                              napi_async_work work) { return napi_ok; }
NAPI_EXTERN napi_status napi_cancel_async_work(napi_env env,
                                               napi_async_work work) { return napi_ok; }

// Methods for custom handling of async operations
NAPI_EXTERN napi_status napi_async_init(napi_env env,
                                        napi_value async_resource,
                                        napi_value async_resource_name,
                                        napi_async_context *result) { return napi_ok; }

NAPI_EXTERN napi_status napi_async_destroy(napi_env env,
                                           napi_async_context async_context) { return napi_ok; }

NAPI_EXTERN napi_status napi_make_callback(napi_env env,
                                           napi_async_context async_context,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value *argv,
                                           napi_value *result) { return napi_ok; }

// version management
NAPI_EXTERN napi_status napi_get_version(napi_env env, uint32_t *result) { return napi_ok; }

NAPI_EXTERN
napi_status napi_get_node_version(napi_env env,
                                  const napi_node_version **version) { return napi_ok; }

// Promises
NAPI_EXTERN napi_status napi_create_promise(napi_env env,
                                            napi_deferred *deferred,
                                            napi_value *promise) { return napi_ok; }
NAPI_EXTERN napi_status napi_resolve_deferred(napi_env env,
                                              napi_deferred deferred,
                                              napi_value resolution) { return napi_ok; }
NAPI_EXTERN napi_status napi_reject_deferred(napi_env env,
                                             napi_deferred deferred,
                                             napi_value rejection) { return napi_ok; }
NAPI_EXTERN napi_status napi_is_promise(napi_env env,
                                        napi_value promise,
                                        bool *is_promise) { return napi_ok; }

// Memory management
NAPI_EXTERN napi_status napi_adjust_external_memory(napi_env env,
                                                    int64_t change_in_bytes,
                                                    int64_t *adjusted_value) { return napi_ok; }

// Runnig a script
NAPI_EXTERN napi_status napi_run_script(napi_env env,
                                        napi_value script,
                                        napi_value *result) { return napi_ok; }

#if NAPI_VERSION >= 3

NAPI_EXTERN napi_status napi_add_env_cleanup_hook(napi_env env,
                                                  void (*fun)(void* arg),
                                                  void* arg) { return napi_ok; }

NAPI_EXTERN napi_status napi_remove_env_cleanup_hook(napi_env env,
                                                     void (*fun)(void* arg),
                                                     void* arg) { return napi_ok; }

#endif  // NAPI_VERSION >= 3

EXTERN_C_END
#endif
