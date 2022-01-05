#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "Listener.h"
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

// CBLDatabase_Close
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

  if (!didClose)
  {
    napi_throw_error(env, "", "Error closing database\n");
  }

  CBLDatabase_Release(database);

  napi_get_boolean(env, didClose, &res);
  return res;
}

// CBLDatabase_Delete
napi_value Database_Delete(napi_env env, napi_callback_info info)
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

  bool didClose = CBLDatabase_Delete(database, &err);

  if (!didClose)
  {
    napi_throw_error(env, "", "Error closing database\n");
  }

  CBLDatabase_Release(database);

  napi_get_boolean(env, didClose, &res);
  return res;
}

struct ChangedDocs
{
  FLString *docIDs;
  unsigned numDocs;
};

static void DatabaseChangeListener(void *cb, const CBLDatabase *db, unsigned numDocs, FLString docIDs[])
{
  struct ChangedDocs *data = malloc(sizeof(numDocs) + sizeof(*docIDs));
  data->docIDs = docIDs;
  data->numDocs = numDocs;

  assert(napi_acquire_threadsafe_function((napi_threadsafe_function)cb) == napi_ok);
  assert(napi_call_threadsafe_function((napi_threadsafe_function)cb, data, napi_tsfn_nonblocking) == napi_ok);
  assert(napi_release_threadsafe_function((napi_threadsafe_function)cb, napi_tsfn_release) == napi_ok);
}

static void DatabaseChangeListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  assert(napi_get_undefined(env, &undefined) == napi_ok);

  struct ChangedDocs changedDocsData = *(struct ChangedDocs *)data;

  napi_value args[1];
  napi_value docIds;

  assert(napi_create_array_with_length(env, changedDocsData.numDocs, &docIds) == napi_ok);

  for (unsigned int i = 0; i < changedDocsData.numDocs; i++)
  {
    napi_value docID;
    assert(napi_create_string_utf8(env, changedDocsData.docIDs[i].buf, changedDocsData.docIDs[i].size, &docID) == napi_ok);
    assert(napi_set_element(env, docIds, i, docID) == napi_ok);
  }

  args[0] = docIds;

  assert(napi_call_function(env, undefined, js_cb, 1, args, NULL) == napi_ok);

  free(data);
}

// CBLDatabase_AddChangeListener
napi_value Database_AddChangeListener(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[2];
  assert(napi_get_cb_info(env, info, &argc, args, NULL, NULL) == napi_ok);

  CBLDatabase *database;
  assert(napi_get_value_external(env, args[0], (void *)&database) == napi_ok);

  napi_value async_resource_name;
  assert(napi_create_string_utf8(env,
                                 "couchbase-lite database change listener",
                                 NAPI_AUTO_LENGTH,
                                 &async_resource_name) == napi_ok);

  napi_threadsafe_function listenerCallback;
  assert(napi_create_threadsafe_function(env, args[1], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, DatabaseChangeListenerCallJS, &listenerCallback) == napi_ok);
  assert(napi_unref_threadsafe_function(env, listenerCallback) == napi_ok);

  CBLListenerToken *token = CBLDatabase_AddChangeListener(database, DatabaseChangeListener, listenerCallback);

  if (!token)
  {
    napi_throw_error(env, "", "Error adding change listener\n");
  }

  struct StopListenerData *stopListenerData = newStopListenerData(listenerCallback, token);
  napi_value stopListener;
  assert(napi_create_function(env, "stopDatabaseChangeListener", NAPI_AUTO_LENGTH, StopChangeListener, stopListenerData, &stopListener) == napi_ok);

  return stopListener;
}
