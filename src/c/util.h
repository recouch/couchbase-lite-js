#pragma once
#include <node_api.h>
#include "cbl/CouchbaseLite.h"

#define CHECK(expr)                                                                 \
  {                                                                                 \
    napi_status _napiStatus = (expr);                                               \
    if (_napiStatus != napi_ok)                                                     \
    {                                                                               \
      fprintf(stderr, "%s:%d: failed assertion `%s'\n", __FILE__, __LINE__, #expr); \
      fflush(stderr);                                                               \
    }                                                                               \
  }

typedef struct ExternalBlobRef
{
  CBLBlob *blob;
  bool releaseOnFinalize;
} external_blob_ref;

typedef struct ExternalBlobReadStreamRef
{
  CBLBlobReadStream *stream;
  bool isOpen;
} external_blob_read_stream_ref;

typedef struct ExternalBlobWriteStreamRef
{
  CBLBlobWriteStream *stream;
  bool isOpen;
} external_blob_write_stream_ref;

typedef struct ExternalDatabaseRef
{
  CBLDatabase *database;
  bool isOpen;
} external_database_ref;

typedef struct ExternalDocumentRef
{
  CBLDocument *document;
} external_document_ref;

typedef struct ExternalQueryRef
{
  CBLQuery *query;
} external_query_ref;

typedef struct ReplicationCallbacks
{
  napi_threadsafe_function conflictResolver;
  napi_threadsafe_function pullFilter;
  napi_threadsafe_function pushFilter;
} replication_callbacks;

typedef struct ExternalReplicatorRef
{
  CBLReplicator *replicator;
} external_replicator_ref;

void assertType(napi_env env, napi_value value, napi_valuetype type, char *errorMsg);
external_blob_ref *createExternalBlobRef(CBLBlob *blob, bool releaseOnFinalize);
external_blob_read_stream_ref *createExternalBlobReadStreamRef(CBLBlobReadStream *stream);
external_blob_write_stream_ref *createExternalBlobWriteStreamRef(CBLBlobWriteStream *stream);
external_database_ref *createExternalDatabaseRef(CBLDatabase *database);
external_document_ref *createExternalDocumentRef(CBLDocument *document);
external_query_ref *createExternalQueryRef(CBLQuery *query);
external_replicator_ref *createExternalReplicatorRef(CBLReplicator *replicator);
bool isDev();
bool isProd();
bool isTest();
char *napiValueToLongString(napi_env env, napi_value value, size_t *str_size);
void logToFile(char *line);
void logIntToFile(int32_t line);
void logFloatToFile(double line);
void logFLStringToFile(FLString line);
void throwCBLError(napi_env env, CBLError err);
