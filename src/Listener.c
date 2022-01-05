#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"

struct StopListenerData
{
  napi_threadsafe_function callback;
  CBLListenerToken *token;
};

// CBLListener_Remove
napi_value Listener_Remove(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  assert(napi_get_cb_info(env, info, &argc, args, NULL, NULL) == napi_ok);

  CBLListenerToken *token;
  assert(napi_get_value_external(env, args[0], (void *)&token) == napi_ok);

  CBLListener_Remove(token);

  napi_value res;
  assert(napi_get_boolean(env, true, &res) == napi_ok);

  return res;
}

struct StopListenerData *newStopListenerData(napi_threadsafe_function callback, CBLListenerToken *token)
{
  struct StopListenerData *stopListenerData = malloc(sizeof(callback) + sizeof(token));
  stopListenerData->callback = callback;
  stopListenerData->token = token;

  return stopListenerData;
}

napi_value StopChangeListener(napi_env env, napi_callback_info info)
{

  struct StopListenerData *data;
  assert(napi_get_cb_info(env, info, NULL, NULL, NULL, (void *)&data) == napi_ok);

  CBLListener_Remove(data->token);
  assert(napi_release_threadsafe_function(data->callback, napi_tsfn_abort) == napi_ok);

  napi_value res;
  assert(napi_get_boolean(env, true, &res) == napi_ok);

  free(data);

  return res;
}
