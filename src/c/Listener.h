#pragma once
#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "util.h"

typedef struct StopListenerData
{
  napi_threadsafe_function callback;
  bool isListening;
  CBLListenerToken *token;
} stop_listener_data;

napi_value Listener_Remove(napi_env env, napi_callback_info info);
struct StopListenerData *newStopListenerData(napi_threadsafe_function callback, CBLListenerToken *token);
napi_value StopChangeListener(napi_env env, napi_callback_info info);
