#include "NapiConvert.h"

bool isArray(napi_env env, napi_value value)
{
  bool isArray;
  CHECK(napi_is_array(env, value, &isArray));

  return isArray;
}

bool napiValueToCBool(napi_env env, napi_value value)
{
  bool res;
  CHECK(napi_get_value_bool(env, value, &res));

  return res;
}

double napiValueToCDouble(napi_env env, napi_value value)
{
  double res;
  CHECK(napi_get_value_double(env, value, &res));

  return res;
}

int32_t napiValueToCInt32(napi_env env, napi_value value)
{
  int32_t res;
  CHECK(napi_get_value_int32(env, value, &res));

  return res;
}

int64_t napiValueToCInt64(napi_env env, napi_value value)
{
  int64_t res;
  bool lossless;
  CHECK(napi_get_value_bigint_int64(env, value, &res, &lossless));

  // TODO: throw error when lossy?

  return res;
}

FLString napiValueToFLString(napi_env env, napi_value value)
{
  size_t str_size;
  CHECK(napi_get_value_string_utf8(env, value, NULL, 0, &str_size));

  char *string;
  str_size = str_size + 1;
  string = (char *)calloc(str_size, sizeof(char));
  CHECK(napi_get_value_string_utf8(env, value, string, str_size, NULL));

  return FLStr(string);
}

FLMutableDict napiValueToFLDict(napi_env env, napi_value object)
{
  FLMutableDict res = FLMutableDict_New();

  napi_value propertyNames;
  CHECK(napi_get_property_names(env, object, &propertyNames));

  uint32_t propertyCount;
  CHECK(napi_get_array_length(env, propertyNames, &propertyCount));

  for (uint32_t i = 0; i < propertyCount; i++)
  {
    napi_value napiKey;
    CHECK(napi_get_element(env, propertyNames, i, &napiKey));
    FLString key = napiValueToFLString(env, napiKey);

    napi_value napiValue;
    CHECK(napi_get_property(env, object, napiKey, &napiValue));

    napi_valuetype type;
    CHECK(napi_typeof(env, napiValue, &type));

    // napi_undefined, napi_symbol, napi_function, and napi_external are not supported and will be ignored
    switch (type)
    {
    case napi_null:
      FLMutableDict_SetNull(res, key);
      break;
    case napi_boolean:
      FLMutableDict_SetBool(res, key, napiValueToCBool(env, napiValue));
      break;
    case napi_number:
    {
      int32_t asInt = napiValueToCInt32(env, napiValue);
      double asDouble = napiValueToCDouble(env, napiValue);

      if (asDouble == asInt)
      {
        FLMutableDict_SetInt(res, key, asInt);
      }
      else
      {
        FLMutableDict_SetDouble(res, key, asDouble);
      }
    }
    break;
    case napi_string:
      FLMutableDict_SetString(res, key, napiValueToFLString(env, napiValue));
      break;
    case napi_object:
      if (isArray(env, napiValue))
      {
        FLMutableArray arrayValue = napiValueToFLArray(env, napiValue);
        FLMutableDict_SetArray(res, key, arrayValue);
        FLMutableArray_Release(arrayValue);
      }
      else
      {
        FLMutableDict dictValue = napiValueToFLDict(env, napiValue);
        FLMutableDict_SetDict(res, key, dictValue);
        FLMutableDict_Release(dictValue);
      }
      break;
    case napi_bigint:
      FLMutableDict_SetInt(res, key, napiValueToCInt64(env, napiValue));
      break;
    case napi_undefined:
    case napi_symbol:
    case napi_function:
    case napi_external:
      // napi_undefined, napi_symbol, napi_function, and napi_external are not supported and will be ignored
      break;
    }
  }

  return res;
}

FLMutableArray napiValueToFLArray(napi_env env, napi_value array)
{
  FLMutableArray res = FLMutableArray_New();
  uint32_t arrayLength;
  CHECK(napi_get_array_length(env, array, &arrayLength));

  for (uint32_t i = 0; i < arrayLength; i++)
  {
    napi_value napiValue;
    CHECK(napi_get_element(env, array, i, &napiValue));

    napi_valuetype type;
    CHECK(napi_typeof(env, napiValue, &type));

    switch (type)
    {
    case napi_null:
      FLMutableArray_AppendNull(res);
      break;
    case napi_boolean:
      FLMutableArray_AppendBool(res, napiValueToCBool(env, napiValue));
      break;
    case napi_number:
    {
      int32_t asInt = napiValueToCInt32(env, napiValue);
      double asDouble = napiValueToCDouble(env, napiValue);

      if (asDouble == asInt)
      {
        FLMutableArray_AppendInt(res, asInt);
      }
      else
      {
        FLMutableArray_AppendDouble(res, asDouble);
      }
    }
    break;
    case napi_string:
      FLMutableArray_AppendString(res, napiValueToFLString(env, napiValue));
      break;
    case napi_object:
      if (isArray(env, napiValue))
      {
        FLMutableArray arrayValue = napiValueToFLArray(env, napiValue);
        FLMutableArray_AppendArray(res, arrayValue);
        FLMutableArray_Release(arrayValue);
      }
      else
      {
        FLMutableDict dictValue = napiValueToFLDict(env, napiValue);
        FLMutableArray_AppendDict(res, dictValue);
        FLMutableDict_Release(dictValue);
      }
      break;
    case napi_bigint:
      FLMutableArray_AppendInt(res, napiValueToCInt64(env, napiValue));
      break;
    case napi_undefined:
    case napi_symbol:
    case napi_function:
    case napi_external:
      // napi_undefined, napi_symbol, napi_function, and napi_external are not supported and will be ignored
      break;
    }
  }

  return res;
}

napi_value flDictToNapiValue(napi_env env, FLDict dict)
{
  napi_value res;
  CHECK(napi_create_object(env, &res));

  FLDictIterator iter;
  FLDictIterator_Begin(dict, &iter);
  FLValue value;

  while (NULL != (value = FLDictIterator_GetValue(&iter)))
  {
    FLString key = FLDictIterator_GetKeyString(&iter);
    FLValue value = FLDictIterator_GetValue(&iter);
    FLValueType type = FLValue_GetType(value);

    napi_value napiKey;
    napi_value napiValue;

    CHECK(napi_create_string_utf8(env, key.buf, key.size, &napiKey));

    switch (type)
    {
    case kFLUndefined:
      CHECK(napi_get_undefined(env, &napiValue));
      break;
    case kFLNull:
      CHECK(napi_get_null(env, &napiValue));
      break;
    case kFLBoolean:
      CHECK(napi_get_boolean(env, FLValue_AsBool(value), &napiValue));
      break;
    case kFLNumber:
      if (FLValue_IsInteger(value))
      {
        if (FLValue_IsUnsigned(value))
        {
          int64_t as64 = FLValue_AsUnsigned(value);
          int32_t as32 = (int32_t)as64;

          if (as32 < as64)
          {
            CHECK(napi_create_bigint_uint64(env, as64, &napiValue));
          }
          else
          {
            CHECK(napi_create_uint32(env, as32, &napiValue));
          }
        }
        else
        {
          int64_t as64 = FLValue_AsInt(value);
          int32_t as32 = (int32_t)as64;

          if (as32 < as64)
          {
            CHECK(napi_create_bigint_int64(env, as64, &napiValue));
          }
          else
          {
            CHECK(napi_create_int32(env, as32, &napiValue));
          }
        }
      }
      else
      {
        CHECK(napi_create_double(env, FLValue_AsDouble(value), &napiValue));
      }
      break;
    case kFLString:
      CHECK(napi_create_string_utf8(env, FLValue_AsString(value).buf, FLValue_AsString(value).size, &napiValue));
      break;
    case kFLData:
      // Unsupported: treat data as null
      CHECK(napi_get_null(env, &napiValue));
      break;
    case kFLArray:
      napiValue = flArrayToNapiValue(env, FLValue_AsArray(value));
      break;
    case kFLDict:
      napiValue = flDictToNapiValue(env, FLValue_AsDict(value));
      break;
    }

    CHECK(napi_set_property(env, res, napiKey, napiValue));

    FLDictIterator_Next(&iter);
  }

  return res;
}

napi_value flArrayToNapiValue(napi_env env, FLArray array)
{
  uint32_t length = FLArray_Count(array);

  napi_value res;
  CHECK(napi_create_array_with_length(env, length, &res));

  FLArrayIterator iter;
  FLArrayIterator_Begin(array, &iter);
  FLValue value;

  while (NULL != (value = FLArrayIterator_GetValue(&iter)))
  {
    FLValue value = FLArrayIterator_GetValue(&iter);
    FLValueType type = FLValue_GetType(value);

    napi_value napiValue;

    switch (type)
    {
    case kFLUndefined:
      CHECK(napi_get_undefined(env, &napiValue));
      break;
    case kFLNull:
      CHECK(napi_get_null(env, &napiValue));
      break;
    case kFLBoolean:
      CHECK(napi_get_boolean(env, FLValue_AsBool(value), &napiValue));
      break;
    case kFLNumber:
      if (FLValue_IsInteger(value))
      {
        if (FLValue_IsUnsigned(value))
        {
          int64_t as64 = FLValue_AsUnsigned(value);
          int32_t as32 = as64;

          if (as32 < as64)
          {
            CHECK(napi_create_bigint_uint64(env, as64, &napiValue));
          }
          else
          {
            CHECK(napi_create_uint32(env, as32, &napiValue));
          }
        }
        else
        {
          int64_t as64 = FLValue_AsInt(value);
          int32_t as32 = as64;

          if (as32 < as64)
          {
            CHECK(napi_create_bigint_int64(env, as64, &napiValue));
          }
          else
          {
            CHECK(napi_create_int32(env, as32, &napiValue));
          }
        }
      }
      else
      {
        CHECK(napi_create_double(env, FLValue_AsDouble(value), &napiValue));
      }
      break;
    case kFLString:
      CHECK(napi_create_string_utf8(env, FLValue_AsString(value).buf, FLValue_AsString(value).size, &napiValue));
      break;
    case kFLData:
      // Unsupported: treat data as null
      CHECK(napi_get_null(env, &napiValue));
      break;
    case kFLArray:
      napiValue = flArrayToNapiValue(env, FLValue_AsArray(value));
      break;
    case kFLDict:
      napiValue = flDictToNapiValue(env, FLValue_AsDict(value));
      break;
    }

    CHECK(napi_set_element(env, res, length - FLArrayIterator_GetCount(&iter), napiValue));

    FLArrayIterator_Next(&iter);
  }

  return res;
}