#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "Listener.h"
#include "util.h"

static void finalize_document_external(napi_env env, void *data, void *hint)
{
  external_document_ref *documentRef = (external_document_ref *)data;

  CBLDocument_Release(documentRef->document);
  free(data);
}

// CBLDocument_ID
napi_value Document_ID(napi_env env, napi_callback_info info)
{

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&docRef));
  CBLDocument *doc = docRef->document;

  FLString id = CBLDocument_ID(doc);

  napi_value res;
  CHECK(napi_create_string_utf8(env, id.buf, id.size, &res));

  return res;
}

// CBLDatabase_GetDocument
napi_value Database_GetDocument(napi_env env, napi_callback_info info)
{
  CBLError err;

  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));
  if (!databaseRef->isOpen)
  {
    napi_throw_error(env, "", "Database is closed");
    return NULL;
  }

  size_t buffer_size = 128;
  char docID[buffer_size];
  CHECK(napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL));

  const CBLDocument *doc = CBLDatabase_GetDocument(databaseRef->database, FLStr(docID), &err);

  napi_value res;
  if (doc)
  {
    external_document_ref *documentRef = createExternalDocumentRef((CBLDocument *)doc);
    CHECK(napi_create_external(env, documentRef, finalize_document_external, NULL, &res));
  }
  else if (err.code == 0)
  {
    CHECK(napi_get_null(env, &res));
  }
  else
  {
    throwCBLError(env, err);
    CHECK(napi_get_null(env, &res));
  }

  return res;
}

// CBLDatabase_GetMutableDocument
napi_value Database_GetMutableDocument(napi_env env, napi_callback_info info)
{
  CBLError err;

  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));
  if (!databaseRef->isOpen)
  {
    napi_throw_error(env, "", "Database is closed");
    return NULL;
  }

  size_t buffer_size = 128;
  char docID[buffer_size];
  napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL);

  CBLDocument *doc = CBLDatabase_GetMutableDocument(databaseRef->database, FLStr(docID), &err);

  napi_value res;
  if (doc)
  {
    external_document_ref *documentRef = createExternalDocumentRef(doc);
    CHECK(napi_create_external(env, documentRef, finalize_document_external, NULL, &res));
  }
  else if (err.code == 0)
  {
    CHECK(napi_get_null(env, &res));
  }
  else
  {
    throwCBLError(env, err);
    CHECK(napi_get_null(env, &res));
  }

  return res;
}

// CBLDatabase_DeleteDocument
napi_value Database_DeleteDocument(napi_env env, napi_callback_info info)
{
  CBLError err;

  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));
  if (!databaseRef->isOpen)
  {
    napi_throw_error(env, "", "Database is closed");
    return NULL;
  }

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[1], (void *)&docRef));
  CBLDocument *doc = docRef->document;

  bool didDelete = CBLDatabase_DeleteDocument(databaseRef->database, doc, &err);

  if (!didDelete)
  {
    napi_throw_error(env, "", "Error deleting document");
  }

  napi_value res;
  napi_get_boolean(env, didDelete, &res);

  return res;
}

// CBLDatabase_SaveDocument
napi_value Database_SaveDocument(napi_env env, napi_callback_info info)
{
  CBLError err;

  napi_value res;
  napi_get_boolean(env, false, &res);

  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));
  if (!databaseRef->isOpen)
  {
    napi_throw_error(env, "", "Database is closed");
    return NULL;
  }
  if (!databaseRef->isOpen)
  {
    napi_throw_error(env, "", "Database is closed");
    return NULL;
  }

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[1], (void *)&docRef));
  CBLDocument *doc = docRef->document;

  bool didSave = CBLDatabase_SaveDocument(databaseRef->database, doc, &err);
  napi_get_boolean(env, didSave, &res);

  if (!didSave)
  {
    throwCBLError(env, err);
  }

  return res;
}

// CBLDocument_Create
napi_value Document_Create(napi_env env, napi_callback_info info)
{
  CBLDocument *doc = CBLDocument_Create();

  napi_value res;
  external_document_ref *documentRef = createExternalDocumentRef(doc);
  CHECK(napi_create_external(env, documentRef, finalize_document_external, NULL, &res));

  return res;
}

// CBLDocument_CreateWithID
napi_value Document_CreateWithID(napi_env env, napi_callback_info info)
{

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  napi_valuetype valuetype0;
  CHECK(napi_typeof(env, args[0], &valuetype0));

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
  external_document_ref *documentRef = createExternalDocumentRef(doc);
  CHECK(napi_create_external(env, documentRef, finalize_document_external, NULL, &res));

  return res;
}

// CBLDocument_CreateJSON
napi_value Document_CreateJSON(napi_env env, napi_callback_info info)
{

  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&docRef));
  CBLDocument *doc = docRef->document;

  FLSliceResult docJson = CBLDocument_CreateJSON(doc);
  napi_value json;
  CHECK(napi_create_string_utf8(env, docJson.buf, docJson.size, &json));

  FLSliceResult_Release(docJson);

  return json;
}

// CBLDocument_SetJSON
napi_value Document_SetJSON(napi_env env, napi_callback_info info)
{

  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&docRef));
  CBLDocument *doc = docRef->document;

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
  CHECK(napi_get_boolean(env, didSetJson, &res));

  return res;
}

static void DocumentChangeListener(void *cb, const CBLDatabase *db, FLString docID)
{
  char *data;
  data = malloc(docID.size + 1);
  assert(FLSlice_ToCString(docID, data, docID.size + 1) == true);

  CHECK(napi_acquire_threadsafe_function((napi_threadsafe_function)cb));
  CHECK(napi_call_threadsafe_function((napi_threadsafe_function)cb, data, napi_tsfn_nonblocking));
  CHECK(napi_release_threadsafe_function((napi_threadsafe_function)cb, napi_tsfn_release));
}

static void DocumentChangeListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  napi_value args[1];
  napi_value docID;
  CHECK(napi_create_string_utf8(env, (char *)data, NAPI_AUTO_LENGTH, &docID));
  args[0] = docID;

  CHECK(napi_call_function(env, undefined, js_cb, 1, args, NULL));

  free(data);
}

// CBLDatabase_AddChangeListener
napi_value Database_AddDocumentChangeListener(napi_env env, napi_callback_info info)
{
  size_t argc = 3;
  napi_value args[3];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));
  if (!databaseRef->isOpen)
  {
    napi_throw_error(env, "", "Database is closed");
    return NULL;
  }

  size_t buffer_size = 128;
  char docID[buffer_size];
  CHECK(napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL));

  napi_value async_resource_name;
  CHECK(napi_create_string_utf8(env,
                                "couchbase-lite document change listener",
                                NAPI_AUTO_LENGTH,
                                &async_resource_name));

  napi_threadsafe_function listenerCallback;
  CHECK(napi_create_threadsafe_function(env, args[2], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, DocumentChangeListenerCallJS, &listenerCallback));
  CHECK(napi_unref_threadsafe_function(env, listenerCallback));

  CBLListenerToken *token = CBLDatabase_AddDocumentChangeListener(databaseRef->database, FLStr(docID), DocumentChangeListener, listenerCallback);

  if (!token)
  {
    napi_throw_error(env, "", "Error adding change listener");
  }

  struct StopListenerData *stopListenerData = newStopListenerData(listenerCallback, token);
  napi_value stopListener;
  CHECK(napi_create_function(env, "stopDocumentChangeListener", NAPI_AUTO_LENGTH, StopChangeListener, stopListenerData, &stopListener));

  return stopListener;
}
