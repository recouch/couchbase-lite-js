#include "Database.c"
#include "Document.c"

#define DECLARE_NAPI_METHOD(name, func)     \
  {                                         \
    name, 0, func, 0, 0, 0, napi_default, 0 \
  }

NAPI_MODULE_INIT(/* env, exports */)
{
  napi_property_descriptor desc[] = {
      DECLARE_NAPI_METHOD("Database_Close", Database_Close),
      DECLARE_NAPI_METHOD("Database_Open", Database_Open),
      DECLARE_NAPI_METHOD("Database_GetDocument", Database_GetDocument),
      DECLARE_NAPI_METHOD("Database_SaveDocument", Database_SaveDocument),
      DECLARE_NAPI_METHOD("Database_DeleteDocument", Database_DeleteDocument),
      DECLARE_NAPI_METHOD("Document_Create", Document_Create),
      DECLARE_NAPI_METHOD("Document_CreateWithID", Document_CreateWithID)};

  napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(*desc), desc);
  assert(status == napi_ok);

  return exports;
}
