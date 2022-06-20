#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "NapiConvert.h"
#include "util.h"

static void finalize_blob_external(napi_env env, void *data, void *hint)
{
  external_blob_ref *blobRef = (external_blob_ref *)data;

  if (blobRef->releaseOnFinalize)
  {
    CBLBlob_Release(blobRef->blob);
  }

  free(data);
}

static void finalize_blob_read_stream_external(napi_env env, void *data, void *hint)
{
  external_blob_read_stream_ref *streamRef = (external_blob_read_stream_ref *)data;

  if (streamRef->isOpen)
  {
    CBLBlobReader_Close(streamRef->stream);
  }

  free(data);
}

static void finalize_blob_write_stream_external(napi_env env, void *data, void *hint)
{
  external_blob_write_stream_ref *streamRef = (external_blob_write_stream_ref *)data;

  if (streamRef->isOpen)
  {
    CBLBlobWriter_Close(streamRef->stream);
  }

  free(data);
}

// CBLBlob_CreateWithData
napi_value Blob_CreateWithData(napi_env env, napi_callback_info info)
{
  // args: contentType, buffer
  size_t argc = 4;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  char contentType[128];
  CHECK(napi_get_value_string_utf8(env, args[0], contentType, 128, NULL));

  uint8_t *buffer;
  size_t size;
  CHECK(napi_get_buffer_info(env, args[1], (void **)&buffer, &size));

  FLSliceResult data = FLSliceResult_CreateWith(buffer, size);
  CBLBlob *blob = CBLBlob_CreateWithData(FLStr(contentType), FLSliceResult_AsSlice(data));
  external_blob_ref *blobRef = createExternalBlobRef(blob, true);
  FLSliceResult_Release(data);

  napi_value res;
  CHECK(napi_create_external(env, blobRef, finalize_blob_external, NULL, &res));

  return res;
}

napi_value Document_SetBlob(napi_env env, napi_callback_info info)
{
  // args: documentRef, propertyName, blobRef
  size_t argc = 3;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&docRef));

  char property[128];
  CHECK(napi_get_value_string_utf8(env, args[1], property, 128, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[2], (void *)&blobRef));

  FLMutableDict properties = CBLDocument_MutableProperties(docRef->document);
  FLSlot_SetBlob(FLMutableDict_Set(properties, FLStr(property)), blobRef->blob);

  napi_value res;
  CHECK(napi_get_undefined(env, &res));

  return res;
}

// FLValue_IsBlob
napi_value Document_IsBlob(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&docRef));

  char property[128];
  CHECK(napi_get_value_string_utf8(env, args[1], property, 128, NULL));

  FLDict properties = CBLDocument_Properties(docRef->document);
  FLValue maybeBlob = FLDict_Get(properties, FLStr(property));

  napi_value res;
  bool isBlob = FLValue_IsBlob(maybeBlob);
  CHECK(napi_get_boolean(env, isBlob, &res));

  return res;
}

// FLValue_GetBlob
napi_value Document_GetBlob(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_document_ref *docRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&docRef));

  char property[128];
  CHECK(napi_get_value_string_utf8(env, args[1], property, 128, NULL));

  FLDict properties = CBLDocument_Properties(docRef->document);
  FLValue blobDict = FLDict_Get(properties, FLStr(property));

  const CBLBlob *blob = FLValue_GetBlob(blobDict);
  external_blob_ref *blobRef = createExternalBlobRef((CBLBlob *)blob, false);

  napi_value res;
  CHECK(napi_create_external(env, blobRef, finalize_blob_external, NULL, &res));

  return res;
}

// CBLBlob_Equals
napi_value Blob_Equals(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  external_blob_ref *anotherBlobRef;
  CHECK(napi_get_value_external(env, args[1], (void *)&anotherBlobRef));

  bool equals = CBLBlob_Equals(blobRef->blob, anotherBlobRef->blob);

  napi_value res;
  CHECK(napi_get_boolean(env, equals, &res));

  return res;
}

// CBLBlob_Length
napi_value Blob_Length(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  uint64_t length = CBLBlob_Length(blobRef->blob);

  napi_value res;
  CHECK(napi_create_int64(env, length, &res));

  return res;
}

// CBLBlob_CreateJSON
napi_value Blob_CreateJSON(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  FLStringResult json = CBLBlob_CreateJSON(blobRef->blob);

  napi_value res;
  CHECK(napi_create_string_utf8(env, json.buf, json.size, &res));
  FLSliceResult_Release(json);

  return res;
}

// CBLBlob_Properties
napi_value Blob_Properties(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  FLDict properties = CBLBlob_Properties(blobRef->blob);
  napi_value res = flDictToNapiValue(env, properties);

  return res;
}

// CBLBlob_Digest
napi_value Blob_Digest(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  FLString digest = CBLBlob_Digest(blobRef->blob);

  napi_value res;
  CHECK(napi_create_string_utf8(env, digest.buf, digest.size, &res));

  return res;
}

// CBLBlob_ContentType
napi_value Blob_ContentType(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  FLString contentType = CBLBlob_ContentType(blobRef->blob);

  napi_value res;
  CHECK(napi_create_string_utf8(env, contentType.buf, contentType.size, &res));

  return res;
}

// CBLBlob_Content
napi_value Blob_Content(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  // TODO: handle errors
  // Currently, false errors are being thrown, so we can't handle real ones
  FLSliceResult content = CBLBlob_Content(blobRef->blob, NULL);

  napi_value res;
  CHECK(napi_create_buffer_copy(env, content.size, content.buf, NULL, &res));

  FLSliceResult_Release(content);

  return res;
}

// Read stream
// CBLBlob_OpenContentStream
napi_value Blob_OpenContentStream(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&blobRef));

  CBLError err;
  CBLBlobReadStream *stream = CBLBlob_OpenContentStream(blobRef->blob, &err);

  if (!stream)
  {
    throwCBLError(env, err);
  }

  napi_value res;
  CHECK(napi_create_external(env, createExternalBlobReadStreamRef(stream), finalize_blob_read_stream_external, NULL, &res));

  return res;
}

// CBLBlobReader_Close
napi_value BlobReader_Close(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_read_stream_ref *streamRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&streamRef));

  napi_value res;
  CHECK(napi_get_undefined(env, &res));

  if (!streamRef->isOpen)
  {
    return res;
  }

  CBLBlobReader_Close(streamRef->stream);
  streamRef->isOpen = false;

  return res;
}

// CBLBlobReader_Read
napi_value BlobReader_Read(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_read_stream_ref *streamRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&streamRef));

  unsigned int maxLength;
  CHECK(napi_get_value_uint32(env, args[1], &maxLength));

  if (!maxLength)
  {
    maxLength = 1024;
  }

  CBLError err;
  uint8_t data[maxLength];
  int bytesRead = CBLBlobReader_Read(streamRef->stream, data, maxLength, &err);

  napi_value res;

  if (bytesRead == -1)
  {
    throwCBLError(env, err);

    CHECK(napi_get_undefined(env, &res));

    return res;
  }

  CHECK(napi_create_buffer_copy(env, bytesRead, data, NULL, &res));

  return res;
}

// Write stream
// CBLBlobWriter_Create
napi_value BlobWriter_Create(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  CBLError err;
  CBLBlobWriteStream *stream = CBLBlobWriter_Create(databaseRef->database, &err);

  if (!stream)
  {
    throwCBLError(env, err);
  }

  napi_value res;
  CHECK(napi_create_external(env, createExternalBlobWriteStreamRef(stream), finalize_blob_write_stream_external, NULL, &res));

  return res;
}

// CBLBlobWriter_Write
napi_value BlobWriter_Write(napi_env env, napi_callback_info info)
{
  // args: blobWriteStreamRef, buffer
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_write_stream_ref *streamRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&streamRef));

  uint8_t *buffer;
  size_t size;
  CHECK(napi_get_buffer_info(env, args[1], (void **)&buffer, &size));

  CBLError err;
  bool didWrite = CBLBlobWriter_Write(streamRef->stream, buffer, size, &err);

  if (!didWrite)
  {
    throwCBLError(env, err);
  }

  napi_value res;
  CHECK(napi_get_boolean(env, didWrite, &res));

  return res;
}

// CBLBlobWriter_Close
napi_value BlobWriter_Close(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_blob_write_stream_ref *streamRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&streamRef));

  napi_value res;
  CHECK(napi_get_undefined(env, &res));

  if (!streamRef->isOpen)
  {
    return res;
  }

  CBLBlobWriter_Close(streamRef->stream);
  streamRef->isOpen = false;

  return res;
}

// CBLBlob_CreateWithStream
napi_value Blob_CreateWithStream(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  char contentType[128];
  CHECK(napi_get_value_string_utf8(env, args[0], contentType, 128, NULL));

  external_blob_write_stream_ref *streamRef;
  CHECK(napi_get_value_external(env, args[1], (void *)&streamRef));

  CBLBlob *blob = CBLBlob_CreateWithStream(FLStr(contentType), streamRef->stream);
  external_blob_ref *blobRef = createExternalBlobRef(blob, true);

  napi_value res;
  CHECK(napi_create_external(env, blobRef, finalize_blob_external, NULL, &res));

  return res;
}

// Documentless blobs
// CBLDatabase_GetBlob
napi_value Database_GetBlob(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  FLDict properties = napiValueToFLDict(env, args[1]);

  napi_value res;
  CBLError err;
  const CBLBlob *blob = CBLDatabase_GetBlob(databaseRef->database, properties, &err);

  if (!blob)
  {
    throwCBLError(env, err);

    CHECK(napi_get_undefined(env, &res));

    return res;
  }

  external_blob_ref *blobRef = createExternalBlobRef((CBLBlob *)blob, true);

  CHECK(napi_create_external(env, blobRef, finalize_blob_external, NULL, &res));

  return res;
}

// CBLDatabase_SaveBlob
napi_value Database_SaveBlob(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];

  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&databaseRef));

  external_blob_ref *blobRef;
  CHECK(napi_get_value_external(env, args[1], (void *)&blobRef));

  CBLError err;
  bool didSave = CBLDatabase_SaveBlob(databaseRef->database, blobRef->blob, &err);

  if (!didSave)
  {
    throwCBLError(env, err);
  }

  napi_value res;
  CHECK(napi_get_boolean(env, didSave, &res));

  return res;
}
