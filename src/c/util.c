#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include <string.h>
#include "cbl/CouchbaseLite.h"
#include "util.h"

void assertType(napi_env env, napi_value value, napi_valuetype type, char *errorMsg)
{
  napi_valuetype valuetype;
  napi_status status = napi_typeof(env, value, &valuetype);
  assert(status == napi_ok);

  if (valuetype != type)
  {
    napi_throw_type_error(env, NULL, errorMsg);
  }
}

external_database_ref *createExternalDatabaseRef(CBLDatabase *database)
{
  external_database_ref *databaseRef = malloc(sizeof(*databaseRef));
  databaseRef->database = database;
  databaseRef->isOpen = true;

  return databaseRef;
}

external_document_ref *createExternalDocumentRef(CBLDocument *document)
{
  external_document_ref *documentRef = malloc(sizeof(*documentRef));
  documentRef->document = document;

  return documentRef;
}

external_query_ref *createExternalQueryRef(CBLQuery *query)
{
  external_query_ref *queryRef = malloc(sizeof(*queryRef));
  queryRef->query = query;

  return queryRef;
}

external_replicator_ref *createExternalReplicatorRef(CBLReplicator *replicator)
{
  external_replicator_ref *replicatorRef = malloc(sizeof(*replicatorRef));
  replicatorRef->replicator = replicator;

  return replicatorRef;
}

bool isDev()
{
  return strcmp(getenv("NODE_ENV"), "dev") == 0 || strcmp(getenv("NODE_ENV"), "development") == 0;
}

bool isProd()
{
  return !isDev() && !isTest();
}

bool isTest()
{
  return strcmp(getenv("NODE_ENV"), "test") == 0;
}

void logToFile(char *line)
{
  FILE *f;
  f = fopen("test.log", "a+");
  fprintf(f, "%s", line);
  fclose(f);
}

void throwCBLError(napi_env env, CBLError err)
{
  char code[20];
  sprintf(code, "%d", err.code);

  FLSliceResult errorMsg = CBLError_Message(&err);
  char msg[errorMsg.size + 1];
  assert(FLSlice_ToCString(FLSliceResult_AsSlice(errorMsg), msg, errorMsg.size + 1) == true);

  assert(napi_throw_error(env, code, msg) == napi_ok);
}