/*--------------------------------------------------------------------------------------+
|
|     $Source: napi/node_api_stub.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "napi/node_api.h"
#include <signal.h>
#include <stdlib.h>

#if defined(_WIN32)
#pragma warning(disable : 4100)
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wconstant-conversion"
#endif

#if defined(_WIN32)
#define ABORT_NO_BACKTRACE() raise(SIGABRT)
#else
#define ABORT_NO_BACKTRACE() abort()
#endif

#ifdef BUILD_FOR_NODE
#if defined(_WIN32)
#pragma comment(linker, "/export:napi_acquire_threadsafe_function=node.exe.napi_acquire_threadsafe_function")
#pragma comment(linker, "/export:napi_add_env_cleanup_hook=node.exe.napi_add_env_cleanup_hook")
#pragma comment(linker, "/export:napi_add_finalizer=node.exe.napi_add_finalizer")
#pragma comment(linker, "/export:napi_adjust_external_memory=node.exe.napi_adjust_external_memory")
#pragma comment(linker, "/export:napi_async_destroy=node.exe.napi_async_destroy")
#pragma comment(linker, "/export:napi_async_init=node.exe.napi_async_init")
#pragma comment(linker, "/export:napi_call_function=node.exe.napi_call_function")
#pragma comment(linker, "/export:napi_call_threadsafe_function=node.exe.napi_call_threadsafe_function")
#pragma comment(linker, "/export:napi_cancel_async_work=node.exe.napi_cancel_async_work")
#pragma comment(linker, "/export:napi_close_callback_scope=node.exe.napi_close_callback_scope")
#pragma comment(linker, "/export:napi_close_escapable_handle_scope=node.exe.napi_close_escapable_handle_scope")
#pragma comment(linker, "/export:napi_close_handle_scope=node.exe.napi_close_handle_scope")
#pragma comment(linker, "/export:napi_coerce_to_bool=node.exe.napi_coerce_to_bool")
#pragma comment(linker, "/export:napi_coerce_to_number=node.exe.napi_coerce_to_number")
#pragma comment(linker, "/export:napi_coerce_to_object=node.exe.napi_coerce_to_object")
#pragma comment(linker, "/export:napi_coerce_to_string=node.exe.napi_coerce_to_string")
#pragma comment(linker, "/export:napi_create_array=node.exe.napi_create_array")
#pragma comment(linker, "/export:napi_create_array_with_length=node.exe.napi_create_array_with_length")
#pragma comment(linker, "/export:napi_create_arraybuffer=node.exe.napi_create_arraybuffer")
#pragma comment(linker, "/export:napi_create_async_work=node.exe.napi_create_async_work")
#pragma comment(linker, "/export:napi_create_bigint_int64=node.exe.napi_create_bigint_int64")
#pragma comment(linker, "/export:napi_create_bigint_uint64=node.exe.napi_create_bigint_uint64")
#pragma comment(linker, "/export:napi_create_bigint_words=node.exe.napi_create_bigint_words")
#pragma comment(linker, "/export:napi_create_buffer=node.exe.napi_create_buffer")
#pragma comment(linker, "/export:napi_create_buffer_copy=node.exe.napi_create_buffer_copy")
#pragma comment(linker, "/export:napi_create_dataview=node.exe.napi_create_dataview")
#pragma comment(linker, "/export:napi_create_double=node.exe.napi_create_double")
#pragma comment(linker, "/export:napi_create_error=node.exe.napi_create_error")
#pragma comment(linker, "/export:napi_create_external=node.exe.napi_create_external")
#pragma comment(linker, "/export:napi_create_external_arraybuffer=node.exe.napi_create_external_arraybuffer")
#pragma comment(linker, "/export:napi_create_external_buffer=node.exe.napi_create_external_buffer")
#pragma comment(linker, "/export:napi_create_function=node.exe.napi_create_function")
#pragma comment(linker, "/export:napi_create_int32=node.exe.napi_create_int32")
#pragma comment(linker, "/export:napi_create_int64=node.exe.napi_create_int64")
#pragma comment(linker, "/export:napi_create_object=node.exe.napi_create_object")
#pragma comment(linker, "/export:napi_create_promise=node.exe.napi_create_promise")
#pragma comment(linker, "/export:napi_create_range_error=node.exe.napi_create_range_error")
#pragma comment(linker, "/export:napi_create_reference=node.exe.napi_create_reference")
#pragma comment(linker, "/export:napi_create_string_latin1=node.exe.napi_create_string_latin1")
#pragma comment(linker, "/export:napi_create_string_utf16=node.exe.napi_create_string_utf16")
#pragma comment(linker, "/export:napi_create_string_utf8=node.exe.napi_create_string_utf8")
#pragma comment(linker, "/export:napi_create_symbol=node.exe.napi_create_symbol")
#pragma comment(linker, "/export:napi_create_threadsafe_function=node.exe.napi_create_threadsafe_function")
#pragma comment(linker, "/export:napi_create_type_error=node.exe.napi_create_type_error")
#pragma comment(linker, "/export:napi_create_typedarray=node.exe.napi_create_typedarray")
#pragma comment(linker, "/export:napi_create_uint32=node.exe.napi_create_uint32")
#pragma comment(linker, "/export:napi_define_class=node.exe.napi_define_class")
#pragma comment(linker, "/export:napi_define_properties=node.exe.napi_define_properties")
#pragma comment(linker, "/export:napi_delete_async_work=node.exe.napi_delete_async_work")
#pragma comment(linker, "/export:napi_delete_element=node.exe.napi_delete_element")
#pragma comment(linker, "/export:napi_delete_property=node.exe.napi_delete_property")
#pragma comment(linker, "/export:napi_delete_reference=node.exe.napi_delete_reference")
#pragma comment(linker, "/export:napi_escape_handle=node.exe.napi_escape_handle")
#pragma comment(linker, "/export:napi_fatal_error=node.exe.napi_fatal_error")
#pragma comment(linker, "/export:napi_fatal_exception=node.exe.napi_fatal_exception")
#pragma comment(linker, "/export:napi_get_and_clear_last_exception=node.exe.napi_get_and_clear_last_exception")
#pragma comment(linker, "/export:napi_get_array_length=node.exe.napi_get_array_length")
#pragma comment(linker, "/export:napi_get_arraybuffer_info=node.exe.napi_get_arraybuffer_info")
#pragma comment(linker, "/export:napi_get_boolean=node.exe.napi_get_boolean")
#pragma comment(linker, "/export:napi_get_buffer_info=node.exe.napi_get_buffer_info")
#pragma comment(linker, "/export:napi_get_cb_info=node.exe.napi_get_cb_info")
#pragma comment(linker, "/export:napi_get_dataview_info=node.exe.napi_get_dataview_info")
#pragma comment(linker, "/export:napi_get_element=node.exe.napi_get_element")
#pragma comment(linker, "/export:napi_get_global=node.exe.napi_get_global")
#pragma comment(linker, "/export:napi_get_last_error_info=node.exe.napi_get_last_error_info")
#pragma comment(linker, "/export:napi_get_named_property=node.exe.napi_get_named_property")
#pragma comment(linker, "/export:napi_get_new_target=node.exe.napi_get_new_target")
#pragma comment(linker, "/export:napi_get_node_version=node.exe.napi_get_node_version")
#pragma comment(linker, "/export:napi_get_null=node.exe.napi_get_null")
#pragma comment(linker, "/export:napi_get_property=node.exe.napi_get_property")
#pragma comment(linker, "/export:napi_get_property_names=node.exe.napi_get_property_names")
#pragma comment(linker, "/export:napi_get_prototype=node.exe.napi_get_prototype")
#pragma comment(linker, "/export:napi_get_reference_value=node.exe.napi_get_reference_value")
#pragma comment(linker, "/export:napi_get_threadsafe_function_context=node.exe.napi_get_threadsafe_function_context")
#pragma comment(linker, "/export:napi_get_typedarray_info=node.exe.napi_get_typedarray_info")
#pragma comment(linker, "/export:napi_get_undefined=node.exe.napi_get_undefined")
#pragma comment(linker, "/export:napi_get_uv_event_loop=node.exe.napi_get_uv_event_loop")
#pragma comment(linker, "/export:napi_get_value_bigint_int64=node.exe.napi_get_value_bigint_int64")
#pragma comment(linker, "/export:napi_get_value_bigint_uint64=node.exe.napi_get_value_bigint_uint64")
#pragma comment(linker, "/export:napi_get_value_bigint_words=node.exe.napi_get_value_bigint_words")
#pragma comment(linker, "/export:napi_get_value_bool=node.exe.napi_get_value_bool")
#pragma comment(linker, "/export:napi_get_value_double=node.exe.napi_get_value_double")
#pragma comment(linker, "/export:napi_get_value_external=node.exe.napi_get_value_external")
#pragma comment(linker, "/export:napi_get_value_int32=node.exe.napi_get_value_int32")
#pragma comment(linker, "/export:napi_get_value_int64=node.exe.napi_get_value_int64")
#pragma comment(linker, "/export:napi_get_value_string_latin1=node.exe.napi_get_value_string_latin1")
#pragma comment(linker, "/export:napi_get_value_string_utf16=node.exe.napi_get_value_string_utf16")
#pragma comment(linker, "/export:napi_get_value_string_utf8=node.exe.napi_get_value_string_utf8")
#pragma comment(linker, "/export:napi_get_value_uint32=node.exe.napi_get_value_uint32")
#pragma comment(linker, "/export:napi_get_version=node.exe.napi_get_version")
#pragma comment(linker, "/export:napi_has_element=node.exe.napi_has_element")
#pragma comment(linker, "/export:napi_has_named_property=node.exe.napi_has_named_property")
#pragma comment(linker, "/export:napi_has_own_property=node.exe.napi_has_own_property")
#pragma comment(linker, "/export:napi_has_property=node.exe.napi_has_property")
#pragma comment(linker, "/export:napi_instanceof=node.exe.napi_instanceof")
#pragma comment(linker, "/export:napi_is_array=node.exe.napi_is_array")
#pragma comment(linker, "/export:napi_is_arraybuffer=node.exe.napi_is_arraybuffer")
#pragma comment(linker, "/export:napi_is_buffer=node.exe.napi_is_buffer")
#pragma comment(linker, "/export:napi_is_dataview=node.exe.napi_is_dataview")
#pragma comment(linker, "/export:napi_is_error=node.exe.napi_is_error")
#pragma comment(linker, "/export:napi_is_exception_pending=node.exe.napi_is_exception_pending")
#pragma comment(linker, "/export:napi_is_promise=node.exe.napi_is_promise")
#pragma comment(linker, "/export:napi_is_typedarray=node.exe.napi_is_typedarray")
#pragma comment(linker, "/export:napi_make_callback=node.exe.napi_make_callback")
#pragma comment(linker, "/export:napi_module_register=node.exe.napi_module_register")
#pragma comment(linker, "/export:napi_new_instance=node.exe.napi_new_instance")
#pragma comment(linker, "/export:napi_open_callback_scope=node.exe.napi_open_callback_scope")
#pragma comment(linker, "/export:napi_open_escapable_handle_scope=node.exe.napi_open_escapable_handle_scope")
#pragma comment(linker, "/export:napi_open_handle_scope=node.exe.napi_open_handle_scope")
#pragma comment(linker, "/export:napi_queue_async_work=node.exe.napi_queue_async_work")
#pragma comment(linker, "/export:napi_ref_threadsafe_function=node.exe.napi_ref_threadsafe_function")
#pragma comment(linker, "/export:napi_reference_ref=node.exe.napi_reference_ref")
#pragma comment(linker, "/export:napi_reference_unref=node.exe.napi_reference_unref")
#pragma comment(linker, "/export:napi_reject_deferred=node.exe.napi_reject_deferred")
#pragma comment(linker, "/export:napi_release_threadsafe_function=node.exe.napi_release_threadsafe_function")
#pragma comment(linker, "/export:napi_remove_env_cleanup_hook=node.exe.napi_remove_env_cleanup_hook")
#pragma comment(linker, "/export:napi_remove_wrap=node.exe.napi_remove_wrap")
#pragma comment(linker, "/export:napi_resolve_deferred=node.exe.napi_resolve_deferred")
#pragma comment(linker, "/export:napi_run_script=node.exe.napi_run_script")
#pragma comment(linker, "/export:napi_set_element=node.exe.napi_set_element")
#pragma comment(linker, "/export:napi_set_named_property=node.exe.napi_set_named_property")
#pragma comment(linker, "/export:napi_set_property=node.exe.napi_set_property")
#pragma comment(linker, "/export:napi_strict_equals=node.exe.napi_strict_equals")
#pragma comment(linker, "/export:napi_throw=node.exe.napi_throw")
#pragma comment(linker, "/export:napi_throw_error=node.exe.napi_throw_error")
#pragma comment(linker, "/export:napi_throw_range_error=node.exe.napi_throw_range_error")
#pragma comment(linker, "/export:napi_throw_type_error=node.exe.napi_throw_type_error")
#pragma comment(linker, "/export:napi_typeof=node.exe.napi_typeof")
#pragma comment(linker, "/export:napi_unref_threadsafe_function=node.exe.napi_unref_threadsafe_function")
#pragma comment(linker, "/export:napi_unwrap=node.exe.napi_unwrap")
#pragma comment(linker, "/export:napi_wrap=node.exe.napi_wrap")
#endif

#elif BUILD_FOR_ELECTRON
#if defined(_WIN32)
#pragma comment(linker, "/export:napi_acquire_threadsafe_function=electron.exe.napi_acquire_threadsafe_function")
#pragma comment(linker, "/export:napi_add_env_cleanup_hook=electron.exe.napi_add_env_cleanup_hook")
#pragma comment(linker, "/export:napi_add_finalizer=electron.exe.napi_add_finalizer")
#pragma comment(linker, "/export:napi_adjust_external_memory=electron.exe.napi_adjust_external_memory")
#pragma comment(linker, "/export:napi_async_destroy=electron.exe.napi_async_destroy")
#pragma comment(linker, "/export:napi_async_init=electron.exe.napi_async_init")
#pragma comment(linker, "/export:napi_call_function=electron.exe.napi_call_function")
#pragma comment(linker, "/export:napi_call_threadsafe_function=electron.exe.napi_call_threadsafe_function")
#pragma comment(linker, "/export:napi_cancel_async_work=electron.exe.napi_cancel_async_work")
#pragma comment(linker, "/export:napi_close_callback_scope=electron.exe.napi_close_callback_scope")
#pragma comment(linker, "/export:napi_close_escapable_handle_scope=electron.exe.napi_close_escapable_handle_scope")
#pragma comment(linker, "/export:napi_close_handle_scope=electron.exe.napi_close_handle_scope")
#pragma comment(linker, "/export:napi_coerce_to_bool=electron.exe.napi_coerce_to_bool")
#pragma comment(linker, "/export:napi_coerce_to_number=electron.exe.napi_coerce_to_number")
#pragma comment(linker, "/export:napi_coerce_to_object=electron.exe.napi_coerce_to_object")
#pragma comment(linker, "/export:napi_coerce_to_string=electron.exe.napi_coerce_to_string")
#pragma comment(linker, "/export:napi_create_array=electron.exe.napi_create_array")
#pragma comment(linker, "/export:napi_create_array_with_length=electron.exe.napi_create_array_with_length")
#pragma comment(linker, "/export:napi_create_arraybuffer=electron.exe.napi_create_arraybuffer")
#pragma comment(linker, "/export:napi_create_async_work=electron.exe.napi_create_async_work")
#pragma comment(linker, "/export:napi_create_bigint_int64=electron.exe.napi_create_bigint_int64")
#pragma comment(linker, "/export:napi_create_bigint_uint64=electron.exe.napi_create_bigint_uint64")
#pragma comment(linker, "/export:napi_create_bigint_words=electron.exe.napi_create_bigint_words")
#pragma comment(linker, "/export:napi_create_buffer=electron.exe.napi_create_buffer")
#pragma comment(linker, "/export:napi_create_buffer_copy=electron.exe.napi_create_buffer_copy")
#pragma comment(linker, "/export:napi_create_dataview=electron.exe.napi_create_dataview")
#pragma comment(linker, "/export:napi_create_double=electron.exe.napi_create_double")
#pragma comment(linker, "/export:napi_create_error=electron.exe.napi_create_error")
#pragma comment(linker, "/export:napi_create_external=electron.exe.napi_create_external")
#pragma comment(linker, "/export:napi_create_external_arraybuffer=electron.exe.napi_create_external_arraybuffer")
#pragma comment(linker, "/export:napi_create_external_buffer=electron.exe.napi_create_external_buffer")
#pragma comment(linker, "/export:napi_create_function=electron.exe.napi_create_function")
#pragma comment(linker, "/export:napi_create_int32=electron.exe.napi_create_int32")
#pragma comment(linker, "/export:napi_create_int64=electron.exe.napi_create_int64")
#pragma comment(linker, "/export:napi_create_object=electron.exe.napi_create_object")
#pragma comment(linker, "/export:napi_create_promise=electron.exe.napi_create_promise")
#pragma comment(linker, "/export:napi_create_range_error=electron.exe.napi_create_range_error")
#pragma comment(linker, "/export:napi_create_reference=electron.exe.napi_create_reference")
#pragma comment(linker, "/export:napi_create_string_latin1=electron.exe.napi_create_string_latin1")
#pragma comment(linker, "/export:napi_create_string_utf16=electron.exe.napi_create_string_utf16")
#pragma comment(linker, "/export:napi_create_string_utf8=electron.exe.napi_create_string_utf8")
#pragma comment(linker, "/export:napi_create_symbol=electron.exe.napi_create_symbol")
#pragma comment(linker, "/export:napi_create_threadsafe_function=electron.exe.napi_create_threadsafe_function")
#pragma comment(linker, "/export:napi_create_type_error=electron.exe.napi_create_type_error")
#pragma comment(linker, "/export:napi_create_typedarray=electron.exe.napi_create_typedarray")
#pragma comment(linker, "/export:napi_create_uint32=electron.exe.napi_create_uint32")
#pragma comment(linker, "/export:napi_define_class=electron.exe.napi_define_class")
#pragma comment(linker, "/export:napi_define_properties=electron.exe.napi_define_properties")
#pragma comment(linker, "/export:napi_delete_async_work=electron.exe.napi_delete_async_work")
#pragma comment(linker, "/export:napi_delete_element=electron.exe.napi_delete_element")
#pragma comment(linker, "/export:napi_delete_property=electron.exe.napi_delete_property")
#pragma comment(linker, "/export:napi_delete_reference=electron.exe.napi_delete_reference")
#pragma comment(linker, "/export:napi_escape_handle=electron.exe.napi_escape_handle")
#pragma comment(linker, "/export:napi_fatal_error=electron.exe.napi_fatal_error")
#pragma comment(linker, "/export:napi_fatal_exception=electron.exe.napi_fatal_exception")
#pragma comment(linker, "/export:napi_get_and_clear_last_exception=electron.exe.napi_get_and_clear_last_exception")
#pragma comment(linker, "/export:napi_get_array_length=electron.exe.napi_get_array_length")
#pragma comment(linker, "/export:napi_get_arraybuffer_info=electron.exe.napi_get_arraybuffer_info")
#pragma comment(linker, "/export:napi_get_boolean=electron.exe.napi_get_boolean")
#pragma comment(linker, "/export:napi_get_buffer_info=electron.exe.napi_get_buffer_info")
#pragma comment(linker, "/export:napi_get_cb_info=electron.exe.napi_get_cb_info")
#pragma comment(linker, "/export:napi_get_dataview_info=electron.exe.napi_get_dataview_info")
#pragma comment(linker, "/export:napi_get_element=electron.exe.napi_get_element")
#pragma comment(linker, "/export:napi_get_global=electron.exe.napi_get_global")
#pragma comment(linker, "/export:napi_get_last_error_info=electron.exe.napi_get_last_error_info")
#pragma comment(linker, "/export:napi_get_named_property=electron.exe.napi_get_named_property")
#pragma comment(linker, "/export:napi_get_new_target=electron.exe.napi_get_new_target")
#pragma comment(linker, "/export:napi_get_node_version=electron.exe.napi_get_node_version")
#pragma comment(linker, "/export:napi_get_null=electron.exe.napi_get_null")
#pragma comment(linker, "/export:napi_get_property=electron.exe.napi_get_property")
#pragma comment(linker, "/export:napi_get_property_names=electron.exe.napi_get_property_names")
#pragma comment(linker, "/export:napi_get_prototype=electron.exe.napi_get_prototype")
#pragma comment(linker, "/export:napi_get_reference_value=electron.exe.napi_get_reference_value")
#pragma comment(linker, "/export:napi_get_threadsafe_function_context=electron.exe.napi_get_threadsafe_function_context")
#pragma comment(linker, "/export:napi_get_typedarray_info=electron.exe.napi_get_typedarray_info")
#pragma comment(linker, "/export:napi_get_undefined=electron.exe.napi_get_undefined")
#pragma comment(linker, "/export:napi_get_uv_event_loop=electron.exe.napi_get_uv_event_loop")
#pragma comment(linker, "/export:napi_get_value_bigint_int64=electron.exe.napi_get_value_bigint_int64")
#pragma comment(linker, "/export:napi_get_value_bigint_uint64=electron.exe.napi_get_value_bigint_uint64")
#pragma comment(linker, "/export:napi_get_value_bigint_words=electron.exe.napi_get_value_bigint_words")
#pragma comment(linker, "/export:napi_get_value_bool=electron.exe.napi_get_value_bool")
#pragma comment(linker, "/export:napi_get_value_double=electron.exe.napi_get_value_double")
#pragma comment(linker, "/export:napi_get_value_external=electron.exe.napi_get_value_external")
#pragma comment(linker, "/export:napi_get_value_int32=electron.exe.napi_get_value_int32")
#pragma comment(linker, "/export:napi_get_value_int64=electron.exe.napi_get_value_int64")
#pragma comment(linker, "/export:napi_get_value_string_latin1=electron.exe.napi_get_value_string_latin1")
#pragma comment(linker, "/export:napi_get_value_string_utf16=electron.exe.napi_get_value_string_utf16")
#pragma comment(linker, "/export:napi_get_value_string_utf8=electron.exe.napi_get_value_string_utf8")
#pragma comment(linker, "/export:napi_get_value_uint32=electron.exe.napi_get_value_uint32")
#pragma comment(linker, "/export:napi_get_version=electron.exe.napi_get_version")
#pragma comment(linker, "/export:napi_has_element=electron.exe.napi_has_element")
#pragma comment(linker, "/export:napi_has_named_property=electron.exe.napi_has_named_property")
#pragma comment(linker, "/export:napi_has_own_property=electron.exe.napi_has_own_property")
#pragma comment(linker, "/export:napi_has_property=electron.exe.napi_has_property")
#pragma comment(linker, "/export:napi_instanceof=electron.exe.napi_instanceof")
#pragma comment(linker, "/export:napi_is_array=electron.exe.napi_is_array")
#pragma comment(linker, "/export:napi_is_arraybuffer=electron.exe.napi_is_arraybuffer")
#pragma comment(linker, "/export:napi_is_buffer=electron.exe.napi_is_buffer")
#pragma comment(linker, "/export:napi_is_dataview=electron.exe.napi_is_dataview")
#pragma comment(linker, "/export:napi_is_error=electron.exe.napi_is_error")
#pragma comment(linker, "/export:napi_is_exception_pending=electron.exe.napi_is_exception_pending")
#pragma comment(linker, "/export:napi_is_promise=electron.exe.napi_is_promise")
#pragma comment(linker, "/export:napi_is_typedarray=electron.exe.napi_is_typedarray")
#pragma comment(linker, "/export:napi_make_callback=electron.exe.napi_make_callback")
#pragma comment(linker, "/export:napi_module_register=electron.exe.napi_module_register")
#pragma comment(linker, "/export:napi_new_instance=electron.exe.napi_new_instance")
#pragma comment(linker, "/export:napi_open_callback_scope=electron.exe.napi_open_callback_scope")
#pragma comment(linker, "/export:napi_open_escapable_handle_scope=electron.exe.napi_open_escapable_handle_scope")
#pragma comment(linker, "/export:napi_open_handle_scope=electron.exe.napi_open_handle_scope")
#pragma comment(linker, "/export:napi_queue_async_work=electron.exe.napi_queue_async_work")
#pragma comment(linker, "/export:napi_ref_threadsafe_function=electron.exe.napi_ref_threadsafe_function")
#pragma comment(linker, "/export:napi_reference_ref=electron.exe.napi_reference_ref")
#pragma comment(linker, "/export:napi_reference_unref=electron.exe.napi_reference_unref")
#pragma comment(linker, "/export:napi_reject_deferred=electron.exe.napi_reject_deferred")
#pragma comment(linker, "/export:napi_release_threadsafe_function=electron.exe.napi_release_threadsafe_function")
#pragma comment(linker, "/export:napi_remove_env_cleanup_hook=electron.exe.napi_remove_env_cleanup_hook")
#pragma comment(linker, "/export:napi_remove_wrap=electron.exe.napi_remove_wrap")
#pragma comment(linker, "/export:napi_resolve_deferred=electron.exe.napi_resolve_deferred")
#pragma comment(linker, "/export:napi_run_script=electron.exe.napi_run_script")
#pragma comment(linker, "/export:napi_set_element=electron.exe.napi_set_element")
#pragma comment(linker, "/export:napi_set_named_property=electron.exe.napi_set_named_property")
#pragma comment(linker, "/export:napi_set_property=electron.exe.napi_set_property")
#pragma comment(linker, "/export:napi_strict_equals=electron.exe.napi_strict_equals")
#pragma comment(linker, "/export:napi_throw=electron.exe.napi_throw")
#pragma comment(linker, "/export:napi_throw_error=electron.exe.napi_throw_error")
#pragma comment(linker, "/export:napi_throw_range_error=electron.exe.napi_throw_range_error")
#pragma comment(linker, "/export:napi_throw_type_error=electron.exe.napi_throw_type_error")
#pragma comment(linker, "/export:napi_typeof=electron.exe.napi_typeof")
#pragma comment(linker, "/export:napi_unref_threadsafe_function=electron.exe.napi_unref_threadsafe_function")
#pragma comment(linker, "/export:napi_unwrap=electron.exe.napi_unwrap")
#pragma comment(linker, "/export:napi_wrap=electron.exe.napi_wrap")
#endif

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

EXTERN_C_END
#endif
