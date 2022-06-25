#include "Listener.h"

// CBLListener_Remove
napi_value Listener_Remove(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  CBLListenerToken *token;
  CHECK(napi_get_value_external(env, args[0], (void *)&token));

  CBLListener_Remove(token);

  napi_value res;
  CHECK(napi_get_boolean(env, true, &res));

  return res;
}

stop_listener_data *newStopListenerData(napi_threadsafe_function callback, CBLListenerToken *token)
{
  stop_listener_data *stopListenerData = malloc(sizeof(callback) + sizeof(token) + sizeof(bool));
  stopListenerData->callback = callback;
  stopListenerData->isListening = true;
  stopListenerData->token = token;

  return stopListenerData;
}

napi_value StopChangeListener(napi_env env, napi_callback_info info)
{
  napi_value res;
  CHECK(napi_get_undefined(env, &res));

  stop_listener_data *data;
  CHECK(napi_get_cb_info(env, info, NULL, NULL, NULL, (void *)&data));

  if (data == NULL)
  {
    return res;
  }

  if (!data->isListening)
  {
    return res;
  }

  CBLListener_Remove(data->token);
  data->isListening = false;
  CHECK(napi_release_threadsafe_function(data->callback, napi_tsfn_abort));

  return res;
}
