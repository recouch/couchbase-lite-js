#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "util.c"

// CBLDatabase_Open
napi_value Database_Open(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLError err;
  CBLDatabase *database;

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  assertType(env, args[0], napi_string, "Wrong arguments: database name must be a string");

  size_t buffer_size = 128;
  char dbName[buffer_size];
  napi_get_value_string_utf8(env, args[0], dbName, buffer_size, NULL);

  database = CBLDatabase_Open(FLStr(dbName), NULL, &err);

  if (!database)
  {
    // Error handling.  For brevity, this is truncated in the rest of the snippet
    // and omitted in other doc code snippets
    fprintf(stderr, "Error opening database (%d / %d)\n", err.domain, err.code);
    FLSliceResult msg = CBLError_Message(&err);
    fprintf(stderr, "%.*s\n", (int)msg.size, (const char *)msg.buf);
    FLSliceResult_Release(msg);

    return NULL;
  }

  napi_value res;
  status = napi_create_external(env, database, NULL, NULL, &res);
  assert(status == napi_ok);

  return res;
}

// CBLDatabase_Open
napi_value Database_Close(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLError err;

  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDatabase *database;
  status = napi_get_value_external(env, args[0], (void *)&database);
  assert(status == napi_ok);

  bool didClose = CBLDatabase_Close(database, &err);
  napi_get_boolean(env, didClose, &res);

  if (!didClose)
  {
    napi_throw_error(env, "", "Error closing database\n");
  }

  CBLDatabase_Release(database);

  return res;
}
