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
      // Database lifecycle
      DECLARE_NAPI_METHOD("Database_BeginTransaction", Database_BeginTransaction),
      DECLARE_NAPI_METHOD("Database_Close", Database_Close),
      DECLARE_NAPI_METHOD("Database_Delete", Database_Delete),
      DECLARE_NAPI_METHOD("Database_EndTransaction", Database_EndTransaction),
      DECLARE_NAPI_METHOD("Database_Open", Database_Open),
      DECLARE_NAPI_METHOD("Database_Name", Database_Name),
      DECLARE_NAPI_METHOD("Database_Path", Database_Path),
      DECLARE_NAPI_METHOD("DeleteDatabase", DeleteDatabase),

      // Database change listeners
      DECLARE_NAPI_METHOD("Database_AddChangeListener", Database_AddChangeListener),

      // Database document operations
      DECLARE_NAPI_METHOD("Database_AddDocumentChangeListener", Database_AddDocumentChangeListener),
      DECLARE_NAPI_METHOD("Database_GetDocument", Database_GetDocument),
      DECLARE_NAPI_METHOD("Database_GetMutableDocument", Database_GetMutableDocument),
      DECLARE_NAPI_METHOD("Database_SaveDocument", Database_SaveDocument),
      DECLARE_NAPI_METHOD("Database_DeleteDocument", Database_DeleteDocument),

      // Document operations
      DECLARE_NAPI_METHOD("Document_Create", Document_Create),
      DECLARE_NAPI_METHOD("Document_CreateWithID", Document_CreateWithID),
      DECLARE_NAPI_METHOD("Document_CreateJSON", Document_CreateJSON),
      DECLARE_NAPI_METHOD("Document_ID", Document_ID),
      DECLARE_NAPI_METHOD("Document_Properties", Document_Properties),
      DECLARE_NAPI_METHOD("Document_SetJSON", Document_SetJSON),
      DECLARE_NAPI_METHOD("Document_SetProperties", Document_SetProperties),

      // Query
      DECLARE_NAPI_METHOD("Database_CreateQuery", Database_CreateQuery),
      DECLARE_NAPI_METHOD("Query_AddChangeListener", Query_AddChangeListener),
      DECLARE_NAPI_METHOD("Query_Execute", Query_Execute),
      DECLARE_NAPI_METHOD("Query_Explain", Query_Explain),
      DECLARE_NAPI_METHOD("Query_Parameters", Query_Parameters),
      DECLARE_NAPI_METHOD("Query_SetParameters", Query_SetParameters),

      // Replicator
      DECLARE_NAPI_METHOD("Replicator_AddChangeListener", Replicator_AddChangeListener),
      DECLARE_NAPI_METHOD("Replicator_AddDocumentReplicationListener", Replicator_AddDocumentReplicationListener),
      DECLARE_NAPI_METHOD("Replicator_Create", Replicator_Create),
      DECLARE_NAPI_METHOD("Replicator_Config", Replicator_Config),
      DECLARE_NAPI_METHOD("Replicator_IsDocumentPending", Replicator_IsDocumentPending),
      DECLARE_NAPI_METHOD("Replicator_PendingDocumentIDs", Replicator_PendingDocumentIDs),
      DECLARE_NAPI_METHOD("Replicator_SetHostReachable", Replicator_SetHostReachable),
      DECLARE_NAPI_METHOD("Replicator_Start", Replicator_Start),
      DECLARE_NAPI_METHOD("Replicator_Status", Replicator_Status),
      DECLARE_NAPI_METHOD("Replicator_Stop", Replicator_Stop),

      // Blob
      DECLARE_NAPI_METHOD("Blob_Content", Blob_Content),
      DECLARE_NAPI_METHOD("Blob_ContentType", Blob_ContentType),
      DECLARE_NAPI_METHOD("Blob_CreateJSON", Blob_CreateJSON),
      DECLARE_NAPI_METHOD("Blob_CreateWithData", Blob_CreateWithData),
      DECLARE_NAPI_METHOD("Blob_CreateWithStream", Blob_CreateWithStream),
      DECLARE_NAPI_METHOD("Blob_Digest", Blob_Digest),
      DECLARE_NAPI_METHOD("Blob_Equals", Blob_Equals),
      DECLARE_NAPI_METHOD("Blob_Length", Blob_Length),
      DECLARE_NAPI_METHOD("Blob_OpenContentStream", Blob_OpenContentStream),
      DECLARE_NAPI_METHOD("BlobReader_Close", BlobReader_Close),
      DECLARE_NAPI_METHOD("BlobReader_Read", BlobReader_Read),
      DECLARE_NAPI_METHOD("BlobWriter_Close", BlobWriter_Close),
      DECLARE_NAPI_METHOD("BlobWriter_Create", BlobWriter_Create),
      DECLARE_NAPI_METHOD("BlobWriter_Write", BlobWriter_Write),
      DECLARE_NAPI_METHOD("Database_GetBlob", Database_GetBlob),
      DECLARE_NAPI_METHOD("Database_SaveBlob", Database_SaveBlob),
      DECLARE_NAPI_METHOD("Document_GetBlob", Document_GetBlob),
      DECLARE_NAPI_METHOD("Document_IsBlob", Document_IsBlob),
      DECLARE_NAPI_METHOD("Document_SetBlob", Document_SetBlob),

      // Constants
      {"CBLJSONLanguage", 0, 0, 0, 0, CBLJSONLanguage, napi_default, 0},
      {"CBLN1QLLanguage", 0, 0, 0, 0, CBLN1QLLanguage, napi_default, 0}};

  napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(*desc), desc);
  assert(status == napi_ok);

  return exports;
}
