#include <assert.h>
#include <node_api.h>
#include <stdio.h>

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