#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"

// CBLListener_Remove
napi_value Listener_Remove(napi_env env, napi_callback_info info)
{
  napi_status status;

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLListenerToken *token;
  status = napi_get_value_external(env, args[0], (void *)&token);
  assert(status == napi_ok);

  CBLListener_Remove(token);

  napi_value res;
  status = napi_get_boolean(env, true, &res);
  assert(status == napi_ok);

  return res;
}
