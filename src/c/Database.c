#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "Listener.h"
#include "util.h"

static void finalize_database_external(napi_env env, void *data, void *hint)
{
  external_database_ref *databaseRef = (external_database_ref *)data;

  if (databaseRef->isOpen)
  {
    CBLDatabase_Close(databaseRef->database, NULL);
  }

  CBLDatabase_Release(databaseRef->database);
  free(data);
}

// CBLDatabase_Open
napi_value Database_Open(napi_env env, napi_callback_info info)
{
  CBLError err;
  CBLDatabase *database;

  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  assertType(env, args[0], napi_string, "Wrong arguments: database name must be a string");

  size_t buffer_size = 128;
  char dbName[buffer_size];
  napi_get_value_string_utf8(env, args[0], dbName, buffer_size, NULL);

  napi_valuetype directoryValueType;
  CHECK(napi_typeof(env, args[1], &directoryValueType));

  if (directoryValueType == napi_string)
  {
    size_t buffer_size = 128;
    char directory[buffer_size];
    napi_get_value_string_utf8(env, args[1], directory, buffer_size, NULL);

    CBLDatabaseConfiguration config;
    config.directory = FLStr(directory);

    database = CBLDatabase_Open(FLStr(dbName), &config, &err);
  }
  else
  {
    database = CBLDatabase_Open(FLStr(dbName), NULL, &err);
  }

  if (!database)
  {
    throwCBLError(env, err);

    return NULL;
  }

  external_database_ref *databaseRef = createExternalDatabaseRef(database);
  napi_value res;
  CHECK(napi_create_external(env, databaseRef, finalize_database_external, NULL, &res));

  return res;
}

// CBLDatabase_Close
napi_value Database_Close(napi_env env, napi_callback_info info)
{
  CBLError err;

  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  bool didClose = CBLDatabase_Close(databaseRef->database, &err);

  if (didClose)
  {
    databaseRef->isOpen = false;
  }
  else
  {
    throwCBLError(env, err);
  }

  napi_get_boolean(env, didClose, &res);
  return res;
}

// CBLDatabase_Delete
napi_value Database_Delete(napi_env env, napi_callback_info info)
{
  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  CBLError err;
  bool didClose = CBLDatabase_Delete(databaseRef->database, &err);
  CHECK(napi_get_boolean(env, didClose, &res));

  if (didClose)
  {
    databaseRef->isOpen = false;
  }
  else
  {
    throwCBLError(env, err);
  }

  return res;
}

// CBL_DeleteDatabase
napi_value DeleteDatabase(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  assertType(env, args[0], napi_string, "Wrong arguments: database name must be a string");

  size_t buffer_size = 128;
  char dbName[buffer_size];
  napi_get_value_string_utf8(env, args[0], dbName, buffer_size, NULL);

  assertType(env, args[1], napi_string, "Wrong arguments: directory must be a string");

  char directory[buffer_size];
  napi_get_value_string_utf8(env, args[1], directory, buffer_size, NULL);

  bool deleted;
  CBLError err;
  deleted = CBL_DeleteDatabase(FLStr(dbName), FLStr(directory), &err);

  if (!deleted)
  {
    throwCBLError(env, err);
  }

  napi_value res;
  CHECK(napi_get_boolean(env, deleted, &res));

  return res;
}

// CBLDatabase_BeginTransaction
napi_value Database_BeginTransaction(napi_env env, napi_callback_info info)
{
  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  CBLError err;
  bool didBegin = CBLDatabase_BeginTransaction(databaseRef->database, &err);

  if (!didBegin)
  {
    throwCBLError(env, err);
    return NULL;
  }

  napi_get_boolean(env, true, &res);
  return res;
}

// CBLDatabase_EndTransaction
napi_value Database_EndTransaction(napi_env env, napi_callback_info info)
{
  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  bool commit;
  CHECK(napi_get_value_bool(env, args[1], &commit));

  CBLError err;
  bool didEnd = CBLDatabase_EndTransaction(databaseRef->database, commit, &err);

  if (!didEnd)
  {
    throwCBLError(env, err);
    return NULL;
  }

  napi_get_boolean(env, true, &res);
  return res;
}

// CBLDatabase_Name
napi_value Database_Name(napi_env env, napi_callback_info info)
{
  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  FLString name = CBLDatabase_Name(databaseRef->database);
  CHECK(napi_create_string_utf8(env, name.buf, name.size, &res));

  return res;
}

// CBLDatabase_Path
napi_value Database_Path(napi_env env, napi_callback_info info)
{
  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  FLStringResult path = CBLDatabase_Path(databaseRef->database);
  CHECK(napi_create_string_utf8(env, path.buf, path.size, &res));
  FLSliceResult_Release(path);

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

  CHECK(napi_acquire_threadsafe_function((napi_threadsafe_function)cb));
  CHECK(napi_call_threadsafe_function((napi_threadsafe_function)cb, data, napi_tsfn_nonblocking));
  CHECK(napi_release_threadsafe_function((napi_threadsafe_function)cb, napi_tsfn_release));
}

static void DatabaseChangeListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  struct ChangedDocs changedDocsData = *(struct ChangedDocs *)data;

  napi_value args[1];
  napi_value docIds;

  CHECK(napi_create_array_with_length(env, changedDocsData.numDocs, &docIds));

  for (unsigned int i = 0; i < changedDocsData.numDocs; i++)
  {
    napi_value docID;
    CHECK(napi_create_string_utf8(env, changedDocsData.docIDs[i].buf, changedDocsData.docIDs[i].size, &docID));
    CHECK(napi_set_element(env, docIds, i, docID));
  }

  args[0] = docIds;

  CHECK(napi_call_function(env, undefined, js_cb, 1, args, NULL));

  free(data);
}

// CBLDatabase_AddChangeListener
napi_value Database_AddChangeListener(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  napi_value async_resource_name;
  CHECK(napi_create_string_utf8(env,
                                "couchbase-lite database change listener",
                                NAPI_AUTO_LENGTH,
                                &async_resource_name));

  napi_threadsafe_function listenerCallback;
  CHECK(napi_create_threadsafe_function(env, args[1], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, DatabaseChangeListenerCallJS, &listenerCallback));
  CHECK(napi_unref_threadsafe_function(env, listenerCallback));

  CBLListenerToken *token = CBLDatabase_AddChangeListener(databaseRef->database, DatabaseChangeListener, listenerCallback);

  if (!token)
  {
    napi_throw_error(env, "", "Error adding change listener\n");
  }

  struct StopListenerData *stopListenerData = newStopListenerData(listenerCallback, token);
  napi_value stopListener;
  CHECK(napi_create_function(env, "stopDatabaseChangeListener", NAPI_AUTO_LENGTH, StopChangeListener, stopListenerData, &stopListener));

  return stopListener;
}
