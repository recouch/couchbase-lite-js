#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "Listener.h"
#include "util.h"

static void finalize_query_external(napi_env env, void *data, void *hint)
{
  external_query_ref *queryRef = (external_query_ref *)data;

  CBLQuery_Release(queryRef->query);
  free(data);
}

// CBLDatabase_CreateQuery
napi_value Database_CreateQuery(napi_env env, napi_callback_info info)
{
  size_t argc = 3;
  napi_value args[argc]; // [database, language, query]

  CBLError err;
  external_database_ref *databaseRef;
  uint32_t language;

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));
  CHECK(napi_get_value_int32(env, args[1], (int32_t *)&language));
  size_t str_size;
  CHECK(napi_get_value_string_utf8(env, args[2], NULL, 0, &str_size));
  char *queryString;
  queryString = (char *)calloc(str_size + 1, sizeof(char));
  str_size = str_size + 1;
  CHECK(napi_get_value_string_utf8(env, args[2], queryString, str_size, NULL));

  CBLQuery *query = CBLDatabase_CreateQuery(databaseRef->database, language, FLStr(queryString), NULL, &err);

  if (!query)
  {
    throwCBLError(env, err);
    return NULL;
  }

  external_query_ref *queryRef = createExternalQueryRef(query);
  napi_value res;
  CHECK(napi_create_external(env, queryRef, finalize_query_external, NULL, &res));

  return res;
}

FLStringResult ResultSet_ToJSON(CBLResultSet *results)
{
  FLMutableArray resultsArray = FLMutableArray_New();

  while (CBLResultSet_Next(results))
  {
    FLDict result = CBLResultSet_ResultDict(results);
    FLMutableArray_AppendDict(resultsArray, result);
  }

  FLStringResult json = FLValue_ToJSON((FLValue)resultsArray);

  FLMutableArray_Release(resultsArray);

  return json;
}

// CBLQuery_Execute
napi_value Query_Execute(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc]; // [database, language, query]

  CBLError err;

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_query_ref *queryRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&queryRef));
  CBLQuery *query = queryRef->query;

  CBLResultSet *results = CBLQuery_Execute(query, &err);

  if (!results)
  {
    throwCBLError(env, err);
    return NULL;
  }

  FLStringResult json = ResultSet_ToJSON(results);
  CBLResultSet_Release(results);
  napi_value res;
  CHECK(napi_create_string_utf8(env, json.buf, json.size, &res));
  FLSliceResult_Release(json);

  return res;
}

// CBLQuery_Explain
napi_value Query_Explain(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc]; // [database, language, query]

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_query_ref *queryRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&queryRef));
  CBLQuery *query = queryRef->query;

  napi_value res;
  FLSliceResult explanation = CBLQuery_Explain(query);
  CHECK(napi_create_string_utf8(env, explanation.buf, explanation.size, &res));

  return res;
}

// CBLQuery_Parameters
napi_value Query_Parameters(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc]; // [database, language, query]

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_query_ref *queryRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&queryRef));
  CBLQuery *query = queryRef->query;

  napi_value res;
  FLDict parameters = CBLQuery_Parameters(query);
  FLSliceResult json = FLValue_ToJSON((FLValue)parameters);
  CHECK(napi_create_string_utf8(env, json.buf, json.size, &res));

  return res;
}

// CBLQuery_SetParameters
napi_value Query_SetParameters(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc]; // [database, language, query]

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_query_ref *queryRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&queryRef));
  CBLQuery *query = queryRef->query;

  size_t str_size;
  napi_get_value_string_utf8(env, args[1], NULL, 0, &str_size);
  char *json;
  json = (char *)calloc(str_size + 1, sizeof(char));
  str_size = str_size + 1;
  napi_get_value_string_utf8(env, args[1], json, str_size, NULL);

  FLDoc parametersDoc = FLDoc_FromJSON(FLStr(json), NULL);

  if (!parametersDoc)
  {
    CHECK(napi_throw_error(env, "", "An error occured"));
    return NULL;
  }

  FLDict parameters = FLValue_AsDict(FLDoc_GetRoot(parametersDoc));

  CBLQuery_SetParameters(query, parameters);

  napi_value res;
  CHECK(napi_get_boolean(env, true, &res));

  return res;
}

static void QueryChangeListener(void *cb, CBLQuery *query, CBLListenerToken *token)
{
  CBLError err;
  CBLResultSet *results = CBLQuery_CopyCurrentResults(query, token, &err);

  if (!results)
  {
    return;
  }

  FLStringResult json = ResultSet_ToJSON(results);
  char *data;
  data = malloc(json.size + 1);
  assert(FLSlice_ToCString(FLSliceResult_AsSlice(json), data, json.size + 1) == true);

  FLSliceResult_Release(json);
  CBLResultSet_Release(results);

  CHECK(napi_acquire_threadsafe_function((napi_threadsafe_function)cb));
  CHECK(napi_call_threadsafe_function((napi_threadsafe_function)cb, data, napi_tsfn_nonblocking));
  CHECK(napi_release_threadsafe_function((napi_threadsafe_function)cb, napi_tsfn_release));
}

static void QueryChangeListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  napi_value args[1];
  napi_value json;
  CHECK(napi_create_string_utf8(env, (char *)data, NAPI_AUTO_LENGTH, &json));
  args[0] = json;

  CHECK(napi_call_function(env, undefined, js_cb, 1, args, NULL));

  free(data);
}

// CBLQuery_AddChangeListener
napi_value Query_AddChangeListener(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc]; // [database, language, query]

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_query_ref *queryRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&queryRef));
  CBLQuery *query = queryRef->query;

  napi_value async_resource_name;
  CHECK(napi_create_string_utf8(env,
                                "couchbase-lite query change listener",
                                NAPI_AUTO_LENGTH,
                                &async_resource_name));
  napi_threadsafe_function listenerCallback;
  CHECK(napi_create_threadsafe_function(env, args[1], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, QueryChangeListenerCallJS, &listenerCallback));
  CHECK(napi_unref_threadsafe_function(env, listenerCallback));

  CBLListenerToken *token = CBLQuery_AddChangeListener(query, QueryChangeListener, listenerCallback);

  if (!token)
  {
    napi_throw_error(env, "", "Error adding change listener");
    CHECK(napi_release_threadsafe_function(listenerCallback, napi_tsfn_abort));
  }

  struct StopListenerData *stopListenerData = newStopListenerData(listenerCallback, token);
  napi_value stopListener;
  CHECK(napi_create_function(env, "stopQueryChangeListener", NAPI_AUTO_LENGTH, StopChangeListener, stopListenerData, &stopListener));

  return stopListener;
}
