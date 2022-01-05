#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"

// CBLDocument_ID
napi_value Document_ID(napi_env env, napi_callback_info info)
{
  napi_status status;

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDocument *doc;
  status = napi_get_value_external(env, args[0], (void *)&doc);
  assert(status == napi_ok);

  FLString id = CBLDocument_ID(doc);

  napi_value res;
  status = napi_create_string_utf8(env, id.buf, id.size, &res);
  assert(status == napi_ok);

  return res;
}

// CBLDatabase_GetDocument
napi_value Database_GetDocument(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLError err;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDatabase *database;
  status = napi_get_value_external(env, args[0], (void *)&database);
  assert(status == napi_ok);

  size_t buffer_size = 128;
  char docID[buffer_size];
  napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL);
  assert(status == napi_ok);

  const CBLDocument *doc = CBLDatabase_GetDocument(database, FLStr(docID), &err);

  napi_value res;
  if (doc)
  {
    assert(napi_create_external(env, (void *)doc, NULL, NULL, &res) == napi_ok);
  }
  else
  {
    assert(napi_get_null(env, &res) == napi_ok);
  }

  return res;
}

// CBLDatabase_GetMutableDocument
napi_value Database_GetMutableDocument(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLError err;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDatabase *database;
  status = napi_get_value_external(env, args[0], (void *)&database);
  assert(status == napi_ok);

  size_t buffer_size = 128;
  char docID[buffer_size];
  napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL);
  assert(status == napi_ok);

  const CBLDocument *doc = CBLDatabase_GetMutableDocument(database, FLStr(docID), &err);

  napi_value res;
  if (doc)
  {
    assert(napi_create_external(env, (void *)doc, NULL, NULL, &res) == napi_ok);
  }
  else
  {
    assert(napi_get_null(env, &res) == napi_ok);
  }

  return res;
}

// CBLDatabase_DeleteDocument
napi_value Database_DeleteDocument(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLError err;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDatabase *database;
  status = napi_get_value_external(env, args[0], (void *)&database);
  assert(status == napi_ok);

  CBLDocument *doc;
  status = napi_get_value_external(env, args[1], (void *)&doc);
  assert(status == napi_ok);

  bool didDelete = CBLDatabase_DeleteDocument(database, doc, &err);

  if (!didDelete)
  {
    napi_throw_error(env, "", "Error deleting document\n");
  }

  CBLDocument_Release(doc);

  napi_value res;
  napi_get_boolean(env, didDelete, &res);

  return res;
}

// CBLDatabase_SaveDocument
napi_value Database_SaveDocument(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLError err;

  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDatabase *database;
  status = napi_get_value_external(env, args[0], (void *)&database);
  assert(status == napi_ok);

  CBLDocument *doc;
  status = napi_get_value_external(env, args[1], (void *)&doc);
  assert(status == napi_ok);

  bool didSave = CBLDatabase_SaveDocument(database, doc, &err);
  napi_get_boolean(env, didSave, &res);

  if (!didSave)
  {
    char errorCode[10];
    sprintf(errorCode, "%d", 42);
    napi_throw_error(env, errorCode, "Error saving document\n");
  }

  return res;
}

// CBLDocument_Create
napi_value Document_Create(napi_env env, napi_callback_info info)
{
  napi_status status;
  CBLDocument *doc = CBLDocument_Create();

  napi_value res;
  status = napi_create_external(env, doc, NULL, NULL, &res);
  assert(status == napi_ok);

  return res;
}

// CBLDocument_CreateWithID
napi_value Document_CreateWithID(napi_env env, napi_callback_info info)
{
  napi_status status;

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  napi_valuetype valuetype0;
  status = napi_typeof(env, args[0], &valuetype0);
  assert(status == napi_ok);

  if (valuetype0 != napi_string)
  {
    napi_throw_type_error(env, NULL, "Wrong arguments");
    return NULL;
  }

  size_t buffer_size = 128;
  char docID[buffer_size];
  napi_get_value_string_utf8(env, args[0], docID, buffer_size, NULL);

  CBLDocument *doc = CBLDocument_CreateWithID(FLStr(docID));

  napi_value res;
  status = napi_create_external(env, doc, NULL, NULL, &res);
  assert(status == napi_ok);

  return res;
}

// CBLDocument_CreateJSON
napi_value Document_CreateJSON(napi_env env, napi_callback_info info)
{
  napi_status status;

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDocument *doc;
  status = napi_get_value_external(env, args[0], (void *)&doc);
  assert(status == napi_ok);

  FLSliceResult docJson = CBLDocument_CreateJSON(doc);
  napi_value json;
  status = napi_create_string_utf8(env, docJson.buf, docJson.size, &json);
  assert(status == napi_ok);

  FLSliceResult_Release(docJson);

  return json;
}

// CBLDocument_SetJSON
napi_value Document_SetJSON(napi_env env, napi_callback_info info)
{
  napi_status status;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDocument *doc;
  status = napi_get_value_external(env, args[0], (void *)&doc);
  assert(status == napi_ok);

  size_t str_size;
  napi_get_value_string_utf8(env, args[1], NULL, 0, &str_size);
  char *json;
  json = (char *)calloc(str_size + 1, sizeof(char));
  str_size = str_size + 1;
  napi_get_value_string_utf8(env, args[1], json, str_size, NULL);

  CBLError err;
  bool didSetJson = CBLDocument_SetJSON(doc, FLStr(json), &err);

  if (!didSetJson)
  {
    napi_throw_error(env, "", "Set JSON failed");
  }

  napi_value res;
  status = napi_get_boolean(env, didSetJson, &res);
  assert(status == napi_ok);

  return res;
}

// CBLDocument_Release
napi_value Document_Release(napi_env env, napi_callback_info info)
{
  napi_status status;

  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  CBLDocument *doc;
  status = napi_get_value_external(env, args[0], (void *)&doc);
  assert(status == napi_ok);

  CBLDocument_Release(doc);

  napi_value res;
  napi_get_boolean(env, true, &res);

  return res;
}

static void DocumentChangeListener(void *func, const CBLDatabase *db, FLString docID)
{
  assert(napi_acquire_threadsafe_function((napi_threadsafe_function)func) == napi_ok);
  assert(napi_call_threadsafe_function((napi_threadsafe_function)func, NULL, napi_tsfn_nonblocking) == napi_ok);
  assert(napi_release_threadsafe_function((napi_threadsafe_function)func, napi_tsfn_release) == napi_ok);
}

static void DocumentChangeListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  assert(napi_get_undefined(env, &undefined) == napi_ok);

  napi_value args[0];

  assert(napi_call_function(env, undefined, js_cb, 1, args, NULL) == napi_ok);

  free(data);
}

// CBLDatabase_AddChangeListener
napi_value
Database_AddDocumentChangeListener(napi_env env, napi_callback_info info)
{
  size_t argc = 3;
  napi_value args[3];
  assert(napi_get_cb_info(env, info, &argc, args, NULL, NULL) == napi_ok);

  CBLDatabase *database;
  assert(napi_get_value_external(env, args[0], (void *)&database) == napi_ok);

  size_t buffer_size = 128;
  char docID[buffer_size];
  assert(napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL) == napi_ok);

  napi_value async_resource_name;
  assert(napi_create_string_utf8(env,
                                 "couchbase-lite database change listener",
                                 NAPI_AUTO_LENGTH,
                                 &async_resource_name) == napi_ok);

  napi_threadsafe_function cb;
  assert(napi_create_threadsafe_function(env, args[2], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, DocumentChangeListenerCallJS, &cb) == napi_ok);
  assert(napi_unref_threadsafe_function(env, cb) == napi_ok);

  CBLListenerToken *token = CBLDatabase_AddDocumentChangeListener(database, FLStr(docID), DocumentChangeListener, cb);

  if (!token)
  {
    napi_throw_error(env, "", "Error adding change listener\n");
  }

  napi_value res;
  assert(napi_create_external(env, token, NULL, NULL, &res) == napi_ok);

  return res;
}
