#include "Blob.c"
#include "Database.c"
#include "Document.c"
#include "Query.c"
#include "Replicator.c"

#define DECLARE_NAPI_METHOD(name, func)     \
  {                                         \
    name, 0, func, 0, 0, 0, napi_default, 0 \
  }

NAPI_MODULE_INIT(/* env, exports */)
{
  napi_value CBLJSONLanguage;
  napi_create_uint32(env, kCBLJSONLanguage, &CBLJSONLanguage);
  napi_value CBLN1QLLanguage;
  napi_create_uint32(env, kCBLN1QLLanguage, &CBLN1QLLanguage);

  napi_property_descriptor desc[] = {
      // Database functions
      DECLARE_NAPI_METHOD("addDatabaseChangeListener", Database_AddChangeListener),
      DECLARE_NAPI_METHOD("beginTransaction", Database_BeginTransaction),
      DECLARE_NAPI_METHOD("closeDatabase", Database_Close),
      DECLARE_NAPI_METHOD("deleteDatabase", Database_Delete),
      DECLARE_NAPI_METHOD("endTransaction", Database_EndTransaction),
      DECLARE_NAPI_METHOD("openDatabase", Database_Open),
      DECLARE_NAPI_METHOD("databaseName", Database_Name),
      DECLARE_NAPI_METHOD("databasePath", Database_Path),

      // Database document operations
      DECLARE_NAPI_METHOD("addDocumentChangeListener", Database_AddDocumentChangeListener),
      DECLARE_NAPI_METHOD("getDocument", Database_GetDocument),
      DECLARE_NAPI_METHOD("getMutableDocument", Database_GetMutableDocument),
      DECLARE_NAPI_METHOD("saveDocument", Database_SaveDocument),
      DECLARE_NAPI_METHOD("deleteDocument", Database_DeleteDocument),

      // Document operations
      DECLARE_NAPI_METHOD("createDocument", Document_Create),
      DECLARE_NAPI_METHOD("getDocumentJSON", Document_CreateJSON),
      DECLARE_NAPI_METHOD("getDocumentID", Document_ID),
      DECLARE_NAPI_METHOD("getDocumentProperties", Document_Properties),
      DECLARE_NAPI_METHOD("setDocumentJSON", Document_SetJSON),
      DECLARE_NAPI_METHOD("setDocumentProperties", Document_SetProperties),

      // Query
      DECLARE_NAPI_METHOD("createQuery", Database_CreateQuery),
      DECLARE_NAPI_METHOD("addQueryChangeListener", Query_AddChangeListener),
      DECLARE_NAPI_METHOD("executeQuery", Query_Execute),
      DECLARE_NAPI_METHOD("explainQuery", Query_Explain),
      DECLARE_NAPI_METHOD("getQueryParameters", Query_Parameters),
      DECLARE_NAPI_METHOD("setQueryParameters", Query_SetParameters),

      // Replicator
      DECLARE_NAPI_METHOD("addDocumentReplicationListener", Replicator_AddDocumentReplicationListener),
      DECLARE_NAPI_METHOD("addReplicatorChangeListener", Replicator_AddChangeListener),
      DECLARE_NAPI_METHOD("createReplicator", Replicator_Create),
      DECLARE_NAPI_METHOD("documentsPendingReplication", Replicator_PendingDocumentIDs),
      DECLARE_NAPI_METHOD("isDocumentPendingReplication", Replicator_IsDocumentPending),
      DECLARE_NAPI_METHOD("replicatorConfiguration", Replicator_Config),
      DECLARE_NAPI_METHOD("replicatorStatus", Replicator_Status),
      DECLARE_NAPI_METHOD("setHostReachable", Replicator_SetHostReachable),
      DECLARE_NAPI_METHOD("startReplicator", Replicator_Start),
      DECLARE_NAPI_METHOD("stopReplicator", Replicator_Stop),

      // Blob
      DECLARE_NAPI_METHOD("blobContent", Blob_Content),
      DECLARE_NAPI_METHOD("blobContentType", Blob_ContentType),
      DECLARE_NAPI_METHOD("blobCreateJson", Blob_CreateJSON),
      DECLARE_NAPI_METHOD("blobProperties", Blob_Properties),
      DECLARE_NAPI_METHOD("createBlobWithData", Blob_CreateWithData),
      DECLARE_NAPI_METHOD("createBlobWithStream", Blob_CreateWithStream),
      DECLARE_NAPI_METHOD("blobDigest", Blob_Digest),
      DECLARE_NAPI_METHOD("blobEquals", Blob_Equals),
      DECLARE_NAPI_METHOD("blobLength", Blob_Length),
      DECLARE_NAPI_METHOD("openBlobContentStream", Blob_OpenContentStream),
      DECLARE_NAPI_METHOD("closeBlobReader", BlobReader_Close),
      DECLARE_NAPI_METHOD("readBlobReader", BlobReader_Read),
      DECLARE_NAPI_METHOD("closeBlobWriter", BlobWriter_Close),
      DECLARE_NAPI_METHOD("createBlobWriter", BlobWriter_Create),
      DECLARE_NAPI_METHOD("writeBlobWriter", BlobWriter_Write),
      DECLARE_NAPI_METHOD("databaseGetBlob", Database_GetBlob),
      DECLARE_NAPI_METHOD("databaseSaveBlob", Database_SaveBlob),
      DECLARE_NAPI_METHOD("documentGetBlob", Document_GetBlob),
      DECLARE_NAPI_METHOD("documentIsBlob", Document_IsBlob),
      DECLARE_NAPI_METHOD("documentSetBlob", Document_SetBlob),

      // Constants
      {"CBLJSONLanguage", 0, 0, 0, 0, CBLJSONLanguage, napi_default, 0},
      {"CBLN1QLLanguage", 0, 0, 0, 0, CBLN1QLLanguage, napi_default, 0}};

  napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(*desc), desc);
  assert(status == napi_ok);

  return exports;
}
