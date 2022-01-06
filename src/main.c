#include "Database.c"
#include "Document.c"
#include "Listener.h"
#include "Query.c"

#define DECLARE_NAPI_METHOD(name, func)     \
  {                                         \
    name, 0, func, 0, 0, 0, napi_default, 0 \
  }

NAPI_MODULE_INIT(/* env, exports */)
{
  napi_property_descriptor desc[] = {
      // Database lifecycle
      DECLARE_NAPI_METHOD("Database_Close", Database_Close),
      DECLARE_NAPI_METHOD("Database_Open", Database_Open),
      DECLARE_NAPI_METHOD("Database_Delete", Database_Delete),

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
      DECLARE_NAPI_METHOD("Document_Release", Document_Release),
      DECLARE_NAPI_METHOD("Document_SetJSON", Document_SetJSON),

      // Listener operations
      DECLARE_NAPI_METHOD("Listener_Remove", Listener_Remove),

      // Queries
      DECLARE_NAPI_METHOD("Database_CreateQuery", Database_CreateQuery),
      DECLARE_NAPI_METHOD("Query_Execute", Query_Execute),
      DECLARE_NAPI_METHOD("Query_Explain", Query_Explain)};

  napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(*desc), desc);
  assert(status == napi_ok);

  return exports;
}
