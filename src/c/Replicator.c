#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "Listener.h"
#include "util.h"

#define CHECK(expr)                                                                 \
  {                                                                                 \
    napi_status status = (expr);                                                    \
    if (status != napi_ok)                                                          \
    {                                                                               \
      fprintf(stderr, "%s:%d: failed assertion `%s'\n", __FILE__, __LINE__, #expr); \
      fflush(stderr);                                                               \
    }                                                                               \
  }

static void finalize_replicator_database_external(napi_env env, void *data, void *hint)
{
  external_database_ref *databaseRef = (external_database_ref *)data;

  CBLDatabase_Release(databaseRef->database);
  free(data);
}

static void finalize_replicator_external(napi_env env, void *data, void *hint)
{
  external_replicator_ref *replicatorRef = (external_replicator_ref *)data;

  CBLReplicator_Release(replicatorRef->replicator);
  free(data);
}

// Lifecycle

// CBLReplicator_Create
napi_value Replicator_Create(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  napi_value napiConfig = args[0];
  CBLReplicatorConfiguration config;
  memset(&config, 0, sizeof(CBLReplicatorConfiguration));

  napi_value napiDatabase;
  CHECK(napi_get_named_property(env, napiConfig, "database", &napiDatabase));
  external_database_ref *databaseRef;
  CHECK(napi_get_value_external(env, napiDatabase, (void *)&databaseRef));
  config.database = databaseRef->database;

  napi_value napiEndpoint;
  napi_valuetype napiEndpointType;
  CHECK(napi_get_named_property(env, napiConfig, "endpoint", &napiEndpoint));
  CHECK(napi_typeof(env, napiEndpoint, &napiEndpointType));

  CBLEndpoint *endpoint = NULL;

  if (napiEndpointType == napi_external)
  {
    CHECK(napi_throw_error(env, "0", "Local DB replication only supported in Enterprise edition"))

    // TODO: Implement localDB replication for Enterprise edition
    // external_database_ref *databaseEndpointRef;
    // CHECK(napi_get_value_external(env, napiEndpoint, (void *)&databaseEndpointRef));
    // endpoint = CBLEndpoint_CreateWithLocalDB(databaseEndpointRef->database);

    return NULL;
  }
  else if (napiEndpointType == napi_string)
  {
    CBLError endpointErr;
    char url[256];
    CHECK(napi_get_value_string_utf8(env, napiEndpoint, url, 256, NULL));
    endpoint = CBLEndpoint_CreateWithURL(FLStr(url), &endpointErr);

    if (!endpoint)
    {
      throwCBLError(env, endpointErr);
      return NULL;
    }
  }
  else
  {
    CHECK(napi_throw_error(env, "0", "Endpoint must be a URL string or a local database reference"))
    return NULL;
  }

  config.endpoint = endpoint;

  napi_value napiReplicatorType;
  napi_valuetype napiReplicatorTypeType;
  CHECK(napi_get_named_property(env, napiConfig, "replicatorType", &napiReplicatorType));
  CHECK(napi_typeof(env, napiReplicatorType, &napiReplicatorTypeType));

  if (napiReplicatorTypeType != napi_undefined)
  {
    char replicatorType[12];
    CHECK(napi_get_value_string_utf8(env, napiReplicatorType, replicatorType, NAPI_AUTO_LENGTH, NULL))

    if (strcmp(replicatorType, "pushAndPull") == 0)
    {
      config.replicatorType = kCBLReplicatorTypePushAndPull;
    }
    else if (strcmp(replicatorType, "push") == 0)
    {
      config.replicatorType = kCBLReplicatorTypePush;
    }
    else if (strcmp(replicatorType, "pull") == 0)
    {
      config.replicatorType = kCBLReplicatorTypePull;
    }
  }

  napi_value continuous;
  napi_valuetype continuousType;
  CHECK(napi_get_named_property(env, napiConfig, "continuous", &continuous));
  CHECK(napi_typeof(env, continuous, &continuousType));

  if (continuousType != napi_undefined)
  {
    CHECK(napi_get_value_bool(env, continuous, &config.continuous));
  }

  napi_value disableAutoPurge;
  napi_valuetype disableAutoPurgeType;
  CHECK(napi_get_named_property(env, napiConfig, "disableAutoPurge", &disableAutoPurge));
  CHECK(napi_typeof(env, disableAutoPurge, &disableAutoPurgeType));

  if (disableAutoPurgeType != napi_undefined)
  {
    CHECK(napi_get_value_bool(env, disableAutoPurge, &config.disableAutoPurge));
  }

  napi_value maxAttempts;
  napi_valuetype maxAttemptsType;
  CHECK(napi_get_named_property(env, napiConfig, "maxAttempts", &maxAttempts));
  CHECK(napi_typeof(env, maxAttempts, &maxAttemptsType));

  if (maxAttemptsType != napi_undefined)
  {
    CHECK(napi_get_value_uint32(env, maxAttempts, &config.maxAttempts));
  }

  napi_value maxAttemptWaitTime;
  napi_valuetype maxAttemptWaitTimeType;
  CHECK(napi_get_named_property(env, napiConfig, "maxAttemptWaitTime", &maxAttemptWaitTime));
  CHECK(napi_typeof(env, maxAttemptWaitTime, &maxAttemptWaitTimeType));

  if (maxAttemptWaitTimeType != napi_undefined)
  {
    CHECK(napi_get_value_uint32(env, maxAttemptWaitTime, &config.maxAttemptWaitTime));
  }

  napi_value heartbeat;
  napi_valuetype heartbeatType;
  CHECK(napi_get_named_property(env, napiConfig, "heartbeat", &heartbeat));
  CHECK(napi_typeof(env, heartbeat, &heartbeatType));

  if (heartbeatType != napi_undefined)
  {
    CHECK(napi_get_value_uint32(env, heartbeat, &config.heartbeat));
  }

  CBLError err;
  CBLReplicator *replicator = CBLReplicator_Create(&config, &err);

  if (!replicator)
  {
    throwCBLError(env, err);
    return NULL;
  }

  napi_value res;
  external_replicator_ref *replicatorRef = createExternalReplicatorRef((CBLReplicator *)replicator);
  CHECK(napi_create_external(env, replicatorRef, finalize_replicator_external, NULL, &res));

  return res;
}

// CBLReplicator_Config
napi_value Replicator_Config(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  const CBLReplicatorConfiguration *config = CBLReplicator_Config(replicatorRef->replicator);

  napi_value replicatorType;
  switch (config->replicatorType)
  {
  case kCBLReplicatorTypePushAndPull:
    CHECK(napi_create_string_utf8(env, "pushAndPull", NAPI_AUTO_LENGTH, &replicatorType));
    break;
  case kCBLReplicatorTypePush:
    CHECK(napi_create_string_utf8(env, "push", NAPI_AUTO_LENGTH, &replicatorType));
    break;
  case kCBLReplicatorTypePull:
    CHECK(napi_create_string_utf8(env, "pull", NAPI_AUTO_LENGTH, &replicatorType));
    break;
  }

  napi_value continuous;
  CHECK(napi_get_boolean(env, config->continuous, &continuous));

  napi_value disableAutoPurge;
  CHECK(napi_get_boolean(env, config->disableAutoPurge, &disableAutoPurge));

  napi_value maxAttempts;
  CHECK(napi_create_int32(env, config->maxAttempts, &maxAttempts));

  napi_value maxAttemptWaitTime;
  CHECK(napi_create_int32(env, config->maxAttemptWaitTime, &maxAttemptWaitTime));

  napi_value heartbeat;
  CHECK(napi_create_int32(env, config->heartbeat, &heartbeat));

  external_database_ref *databaseRef = createExternalDatabaseRef(config->database);
  napi_value externalDatabaseRef;
  CHECK(napi_create_external(env, databaseRef, finalize_replicator_database_external, NULL, &externalDatabaseRef));

  napi_value res;
  CHECK(napi_create_object(env, &res));

  napi_property_descriptor properties[] = {
      {"continuous", 0, 0, 0, 0, continuous, napi_enumerable, 0},
      {"database", 0, 0, 0, 0, externalDatabaseRef, napi_default, 0},
      {"disableAutoPurge", 0, 0, 0, 0, disableAutoPurge, napi_enumerable, 0},
      {"maxAttempts", 0, 0, 0, 0, maxAttempts, napi_enumerable, 0},
      {"maxAttemptWaitTime", 0, 0, 0, 0, maxAttemptWaitTime, napi_enumerable, 0},
      {"heartbeat", 0, 0, 0, 0, heartbeat, napi_enumerable, 0},
      {"replicatorType", 0, 0, 0, 0, replicatorType, napi_enumerable, 0},
  };

  CHECK(napi_define_properties(env, res, sizeof(properties) / sizeof(*properties), properties));

  return res;
}

// CBLReplicator_Start
napi_value Replicator_Start(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  bool resetCheckpoint = false;

  if (args[1])
  {
    CHECK(napi_get_value_bool(env, args[1], &resetCheckpoint));
  }

  CBLReplicator_Start(replicatorRef->replicator, resetCheckpoint);

  napi_value res;
  CHECK(napi_get_boolean(env, true, &res));

  return res;
}

// CBLReplicator_Stop
napi_value Replicator_Stop(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  CBLReplicator_Stop(replicatorRef->replicator);

  napi_value res;
  CHECK(napi_get_boolean(env, true, &res));

  return res;
}

// CBLReplicator_SetHostReachable
napi_value Replicator_SetHostReachable(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  bool reachable;
  CHECK(napi_get_value_bool(env, args[1], &reachable));

  CBLReplicator_SetHostReachable(replicatorRef->replicator, reachable);

  napi_value res;
  CHECK(napi_get_boolean(env, true, &res));

  return res;
}

// CBLReplicator_SetSuspended
napi_value Replicator_SetSuspended(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  bool suspended;
  CHECK(napi_get_value_bool(env, args[1], &suspended));

  CBLReplicator_SetSuspended(replicatorRef->replicator, suspended);

  napi_value res;
  CHECK(napi_get_boolean(env, true, &res));

  return res;
}

// Status and Progress

// CBLReplicator_Status
napi_value Replicator_Status(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  CBLReplicatorStatus replicatorStatus = CBLReplicator_Status(replicatorRef->replicator);
  napi_value activity;
  napi_value error;
  napi_value progress;
  napi_value complete;
  napi_value documentCount;

  switch (replicatorStatus.activity)
  {
  case kCBLReplicatorStopped:
    CHECK(napi_create_string_utf8(env, "stopped", NAPI_AUTO_LENGTH, &activity));
    break;
  case kCBLReplicatorOffline:
    CHECK(napi_create_string_utf8(env, "offline", NAPI_AUTO_LENGTH, &activity));
    break;
  case kCBLReplicatorConnecting:
    CHECK(napi_create_string_utf8(env, "connecting", NAPI_AUTO_LENGTH, &activity));
    break;
  case kCBLReplicatorIdle:
    CHECK(napi_create_string_utf8(env, "idle", NAPI_AUTO_LENGTH, &activity));
    break;
  case kCBLReplicatorBusy:
    CHECK(napi_create_string_utf8(env, "busy", NAPI_AUTO_LENGTH, &activity));
    break;
  }

  FLSliceResult errorMsg = CBLError_Message(&replicatorStatus.error);
  char msg[errorMsg.size + 1];
  assert(FLSlice_ToCString(FLSliceResult_AsSlice(errorMsg), msg, errorMsg.size + 1) == true);
  CHECK(napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &error))

  CHECK(napi_create_object(env, &progress));
  CHECK(napi_create_double(env, replicatorStatus.progress.complete, &complete));
  CHECK(napi_set_named_property(env, progress, "complete", complete));
  CHECK(napi_create_bigint_uint64(env, replicatorStatus.progress.documentCount, &documentCount));
  CHECK(napi_set_named_property(env, progress, "documentCount", documentCount));

  napi_value res;
  CHECK(napi_create_object(env, &res));
  CHECK(napi_set_named_property(env, res, "activity", activity));
  CHECK(napi_set_named_property(env, res, "progress", progress));

  return res;
}

// CBLReplicator_PendingDocumentIDs
napi_value Replicator_PendingDocumentIDs(napi_env env, napi_callback_info info)
{
  return NULL;
}

// CBLReplicator_IsDocumentPending
napi_value Replicator_IsDocumentPending(napi_env env, napi_callback_info info)
{
  return NULL;
}

// CBLReplicator_AddChangeListener
napi_value Replicator_AddChangeListener(napi_env env, napi_callback_info info)
{
  return NULL;
}

// CBLReplicator_AddDocumentReplicationListener
napi_value Replicator_AddDocumentReplicationListener(napi_env env, napi_callback_info info)
{
  return NULL;
}
