#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"

#define CHECK(expr)                                                                 \
  {                                                                                 \
    napi_status status = (expr);                                                    \
    if (status != napi_ok)                                                          \
    {                                                                               \
      fprintf(stderr, "%s:%d: failed assertion `%s'\n", __FILE__, __LINE__, #expr); \
      fflush(stderr);                                                               \
      abort();                                                                      \
    }                                                                               \
  }

// enum QueryLanguage

// CBLDatabase_CreateQuery
napi_value Database_CreateQuery(napi_env env, napi_callback_info info)
{
  size_t argc = 3;
  napi_value args[argc]; // [database, language, query]

  CBLError err;
  CBLDatabase *database;
  uint32_t language;

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  CHECK(napi_get_value_external(env, args[0], (void *)&database));
  CHECK(napi_get_value_int32(env, args[1], (int32_t *)&language));
  size_t str_size;
  CHECK(napi_get_value_string_utf8(env, args[2], NULL, 0, &str_size));
  char *queryString;
  queryString = (char *)calloc(str_size + 1, sizeof(char));
  str_size = str_size + 1;
  CHECK(napi_get_value_string_utf8(env, args[2], queryString, str_size, NULL));

  CBLQuery *query = CBLDatabase_CreateQuery(database, language, FLStr(queryString), NULL, &err);

  napi_value res;
  CHECK(napi_create_external(env, (void *)query, NULL, NULL, &res));

  return res;
}

// CBLQuery_Execute
napi_value Query_Execute(napi_env env, napi_callback_info info)
{
  size_t argc = 3;
  napi_value args[argc]; // [database, language, query]

  CBLError err;
  CBLQuery *query;

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  CHECK(napi_get_value_external(env, args[0], (void *)&query));

  napi_value res;
  CHECK(napi_create_array(env, &res));

  CBLResultSet *results = CBLQuery_Execute(query, &err);
  FLMutableArray resultsArray = FLMutableArray_New();

  while (CBLResultSet_Next(results))
  {
    FLDict result = CBLResultSet_ResultDict(results);
    FLMutableArray_AppendDict(resultsArray, result);
  }

  CBLResultSet_Release(results);

  FLStringResult jsonRes = FLValue_ToJSON((FLValue)resultsArray);

  CHECK(napi_create_string_utf8(env, jsonRes.buf, jsonRes.size, &res));

  FLSliceResult_Release(jsonRes);
  FLMutableArray_Release(resultsArray);

  return res;
}

// CBLQuery_Explain
napi_value Query_Explain(napi_env env, napi_callback_info info)
{
  size_t argc = 3;
  napi_value args[argc]; // [database, language, query]

  CBLQuery *query;

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  CHECK(napi_get_value_external(env, args[0], (void *)&query));

  napi_value res;
  FLSliceResult explanation = CBLQuery_Explain(query);
  CHECK(napi_create_string_utf8(env, explanation.buf, explanation.size, &res));

  return res;
}

// CBLQuery_Parameters
napi_value Query_Parameters(napi_env env, napi_callback_info info)
{
  napi_value res;
  CHECK(napi_get_undefined(env, &res));
  return res;
}

// CBLQuery_SetParameters
napi_value Query_SetParameters(napi_env env, napi_callback_info info)
{
  napi_value res;
  CHECK(napi_get_undefined(env, &res));
  return res;
}

// Change listeners
// CBLQuery_AddChangeListener
napi_value Query_AddChangeListener(napi_env env, napi_callback_info info)
{
  napi_value res;
  CHECK(napi_get_undefined(env, &res));
  return res;
}

// CBLQuery_CopyCurrentResults
napi_value Query_CopyCurrentResults(napi_env env, napi_callback_info info)
{
  napi_value res;
  CHECK(napi_get_undefined(env, &res));
  return res;
}
