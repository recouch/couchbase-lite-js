#pragma once
#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "util.h"

// Napi values to C variables
bool isArray(napi_env env, napi_value value);
bool napiValueToCBool(napi_env env, napi_value value);
double napiValueToCDouble(napi_env env, napi_value value);
int64_t napiValueToCInt64(napi_env env, napi_value value);

// Napi values to Fleece objects
FLString napiValueToFLString(napi_env env, napi_value value);
FLMutableDict napiValueToFLDict(napi_env env, napi_value object);
FLMutableArray napiValueToFLArray(napi_env env, napi_value array);

// Fleece objects to Napi values
napi_value flDictToNapiValue(napi_env env, FLDict dict);
napi_value flArrayToNapiValue(napi_env env, FLArray array);
