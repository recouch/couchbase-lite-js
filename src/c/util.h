#pragma once
#include <node_api.h>
#include "cbl/CouchbaseLite.h"

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

typedef struct ExternalReplicatorRef
{
  CBLReplicator *replicator;
} external_replicator_ref;

void assertType(napi_env env, napi_value value, napi_valuetype type, char *errorMsg);
external_database_ref *createExternalDatabaseRef(CBLDatabase *database);
external_document_ref *createExternalDocumentRef(CBLDocument *document);
external_query_ref *createExternalQueryRef(CBLQuery *query);
external_replicator_ref *createExternalReplicatorRef(CBLReplicator *replicator);
bool isDev();
bool isProd();
bool isTest();
void logToFile(char *line);
void throwCBLError(napi_env env, CBLError err);
