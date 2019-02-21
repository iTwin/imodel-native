#include "node_api.h"

// Warning: Keep in-sync with napi_status enum
static
const char* error_messages[] = {nullptr,
                                "Invalid argument",
                                "An object was expected",
                                "A string was expected",
                                "A string or symbol was expected",
                                "A function was expected",
                                "A number was expected",
                                "A boolean was expected",
                                "An array was expected",
                                "Unknown failure",
                                "An exception is pending",
                                "The async work item was cancelled",
                                "napi_escape_handle already called on scope"};

static inline napi_status napi_clear_last_error(napi_env env) {
  env->last_error.error_code = napi_ok;

  // TODO(boingoing): Should this be a callback?
  env->last_error.engine_error_code = 0;
  env->last_error.engine_reserved = nullptr;
  return napi_ok;
}

static inline
napi_status napi_set_last_error(napi_env env, napi_status error_code,
                                uint32_t engine_error_code,
                                void* engine_reserved) {
  env->last_error.error_code = error_code;
  env->last_error.engine_error_code = engine_error_code;
  env->last_error.engine_reserved = engine_reserved;
  return error_code;
}

napi_status napi_get_last_error_info(napi_env env,
                                     const napi_extended_error_info** result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  // you must update this assert to reference the last message
  // in the napi_status enum each time a new error message is added.
  // We don't have a napi_status_last as this would result in an ABI
  // change each time a message was added.
  static_assert(
      node::arraysize(error_messages) == napi_escape_called_twice + 1,
      "Count of error messages must match count of error values");
  CHECK_LE(env->last_error.error_code, napi_escape_called_twice);

  // Wait until someone requests the last error information to fetch the error
  // message string
  env->last_error.error_message =
      error_messages[env->last_error.error_code];

  *result = &(env->last_error);
  return napi_ok;
}

NAPI_NO_RETURN void napi_fatal_error(const char* location,
                                     size_t location_len,
                                     const char* message,
                                     size_t message_len) {
  std::string location_string;
  std::string message_string;

  if (location_len != NAPI_AUTO_LENGTH) {
    location_string.assign(
        const_cast<char*>(location), location_len);
  } else {
    location_string.assign(
        const_cast<char*>(location), strlen(location));
  }

  if (message_len != NAPI_AUTO_LENGTH) {
    message_string.assign(
        const_cast<char*>(message), message_len);
  } else {
    message_string.assign(
        const_cast<char*>(message), strlen(message));
  }

  node::FatalError(location_string.c_str(), message_string.c_str());
}
