#include <assert.h>
#include <node_api.h>
#include "cbl/CouchbaseLite.h"

//  Purpose-- provide an overview of available crud  and sync functionality
//

static void TestDB(napi_env env, napi_callback_info info)
{
  // Get the database (and create it if it doesn't exist)
  CBLError err;
  CBLDatabase *database = CBLDatabase_Open(FLSTR("mydb"), NULL, &err);
  if (!database)
  {
    // Error handling.  For brevity, this is truncated in the rest of the snippet
    // and omitted in other doc code snippets
    fprintf(stderr, "Error opening database (%d / %d)\n", err.domain, err.code);
    FLSliceResult msg = CBLError_Message(&err);
    fprintf(stderr, "%.*s\n", (int)msg.size, (const char *)msg.buf);
    FLSliceResult_Release(msg);
    return;
  }

  // The lack of 'const' indicates this document is mutable
  // Create a new document (i.e. a record) in the database
  CBLDocument *mutableDoc = CBLDocument_Create();
  FLMutableDict properties = CBLDocument_MutableProperties(mutableDoc);
  FLMutableDict_SetFloat(properties, FLSTR("version"), 3.0f);

  // Save it to the database
  if (!CBLDatabase_SaveDocument(database, mutableDoc, &err))
  {
    // Failed to save, do error handling as above
    return;
  }

  // Since we will release the document, make a copy of the ID since it
  // is an internal pointer.  Whenever we create or get an FLSliceResult
  // or FLStringResult we will need to free it later too!
  FLStringResult id = FLSlice_Copy(CBLDocument_ID(mutableDoc));
  CBLDocument_Release(mutableDoc);

  // Update a document
  mutableDoc = CBLDatabase_GetMutableDocument(database, FLSliceResult_AsSlice(id), &err);
  if (!mutableDoc)
  {
    // Failed to retrieve, do error handling as above.  NOTE: error code 0 simply means
    // the document does not exist.
    return;
  }

  properties = CBLDocument_MutableProperties(mutableDoc);
  FLMutableDict_SetString(properties, FLSTR("language"), FLSTR("C"));
  if (!CBLDatabase_SaveDocument(database, mutableDoc, &err))
  {
    // Failed to save, do error handling as above
    return;
  }

  // Note const here, means readonly
  const CBLDocument *docAgain = CBLDatabase_GetDocument(database, FLSliceResult_AsSlice(id), &err);
  if (!docAgain)
  {
    // Failed to retrieve, do error handling as above.  NOTE: error code 0 simply means
    // the document does not exist.
    return;
  }

  // No copy this time, so no release later (notice it is not FLStringResult this time)
  FLString retrievedID = CBLDocument_ID(docAgain);
  FLDict retrievedProperties = CBLDocument_Properties(docAgain);
  FLString retrievedLanguage = FLValue_AsString(FLDict_Get(retrievedProperties, FLSTR("language")));
  printf("Document ID :: %.*s\n", (int)retrievedID.size, (const char *)retrievedID.buf);
  printf("Learning %.*s\n", (int)retrievedLanguage.size, (const char *)retrievedLanguage.buf);

  CBLDocument_Release(mutableDoc);
  CBLDocument_Release(docAgain);
  FLSliceResult_Release(id);

  // Create a query to fetch documents of type SDK
  int errorPos;
  CBLQuery *query = CBLDatabase_CreateQuery(database, kCBLN1QLLanguage, FLSTR("SELECT * FROM _ WHERE type = \"SDK\""), &errorPos, &err);
  if (!query)
  {
    // Failed to create query, do error handling as above
    // Note that errorPos will contain the position in the N1QL string
    // that the parse failed, if applicable
    return;
  }

  CBLResultSet *result = CBLQuery_Execute(query, &err);
  if (!result)
  {
    // Failed to run query, do error handling as above
    return;
  }

  // TODO: Result set count?
  CBLResultSet_Release(result);
  CBLQuery_Release(query);

  // Create replicator to push and pull changes to and from the cloud
  CBLEndpoint *targetEndpoint = CBLEndpoint_CreateWithURL(FLSTR("ws://localhost:4984/getting-started-db"), &err);
  if (!targetEndpoint)
  {
    // Failed to create endpoint, do error handling as above
    return;
  }

  CBLReplicatorConfiguration replConfig;
  CBLAuthenticator *basicAuth = CBLAuth_CreatePassword(FLSTR("john"), FLSTR("pass"));
  memset(&replConfig, 0, sizeof(replConfig));
  replConfig.database = database;
  replConfig.endpoint = targetEndpoint;
  replConfig.authenticator = basicAuth;

  CBLReplicator *replicator = CBLReplicator_Create(&replConfig, &err);
  CBLAuth_Free(basicAuth);
  CBLEndpoint_Free(targetEndpoint);
  if (!replicator)
  {
    // Failed to create replicator, do error handling as above
    return;
  }

  // Assume a function like the simple following
  //
  // static void getting_started_change_listener(void* context, CBLReplicator* repl, const CBLReplicatorStatus* status) {
  //     if(status->error.code != 0) {
  //         printf("Error %d / %d\n", status->error.domain, status->error.code);
  //     }
  // }

  // CBLListenerToken *token = CBLReplicator_AddChangeListener(replicator, getting_started_change_listener, NULL);

  CBLReplicator_Start(replicator, false);

  // Later, stop and release the replicator
}

#define DECLARE_NAPI_METHOD(name, func)     \
  {                                         \
    name, 0, func, 0, 0, 0, napi_default, 0 \
  }

static napi_value Init(napi_env env, napi_value exports)
{
  napi_status status;
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("test", TestDB);
  status = napi_define_properties(env, exports, 1, &desc);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
