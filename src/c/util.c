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

external_blob_ref *createExternalBlobRef(CBLBlob *blob, bool releaseOnFinalize)
{
  external_blob_ref *blobRef = malloc(sizeof(*blobRef));
  blobRef->blob = blob;
  blobRef->releaseOnFinalize = releaseOnFinalize;

  return blobRef;
}

external_blob_read_stream_ref *createExternalBlobReadStreamRef(CBLBlobReadStream *stream)
{
  external_blob_read_stream_ref *streamRef = malloc(sizeof(*streamRef));
  streamRef->stream = stream;
  streamRef->isOpen = true;

  return streamRef;
}

external_blob_write_stream_ref *createExternalBlobWriteStreamRef(CBLBlobWriteStream *stream)
{
  external_blob_write_stream_ref *streamRef = malloc(sizeof(*streamRef));
  streamRef->stream = stream;
  streamRef->isOpen = true;

  return streamRef;
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

char *napiValueToLongString(napi_env env, napi_value value, size_t *str_size)
{
  CHECK(napi_get_value_string_utf8(env, value, NULL, 0, str_size));

  char *res;
  res = (char *)calloc(*str_size + 1, sizeof(char));
  str_size = str_size + 1;
  CHECK(napi_get_value_string_utf8(env, value, res, *str_size, NULL));

  return res;
}

void logToFile(char *line)
{
  FILE *f;
  f = fopen("test.log", "a+");
  fprintf(f, "%s\n", line);
  fclose(f);
}

void logIntToFile(int32_t line)
{
  FILE *f;
  f = fopen("test.log", "a+");
  fprintf(f, "%i\n", line);
  fclose(f);
}

void logFloatToFile(double line)
{
  FILE *f;
  f = fopen("test.log", "a+");
  fprintf(f, "%f\n", line);
  fclose(f);
}

void logFLStringToFile(FLString line)
{
  FILE *f;
  f = fopen("test.log", "a+");
  fprintf(f, "%.*s\n", (int)line.size, (char *)line.buf);
  fclose(f);
}

void throwCBLError(napi_env env, CBLError err)
{
  char code[20];
  sprintf(code, "%d", err.code);

  FLSliceResult errorMsg = CBLError_Message(&err);
  char msg[errorMsg.size + 1];
  assert(FLSlice_ToCString(FLSliceResult_AsSlice(errorMsg), msg, errorMsg.size + 1) == true);
  FLSliceResult_Release(errorMsg);

  assert(napi_throw_error(env, code, msg) == napi_ok);
}