#pragma once
#include <node_api.h>
#include "cbl/CouchbaseLite.h"

struct StopListenerData
{
  napi_threadsafe_function callback;
  CBLListenerToken *token;
};

napi_value Listener_Remove(napi_env env, napi_callback_info info);
struct StopListenerData *newStopListenerData(napi_threadsafe_function callback, CBLListenerToken *token);
napi_value StopChangeListener(napi_env env, napi_callback_info info);
