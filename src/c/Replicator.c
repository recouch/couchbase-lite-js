#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include "cbl/CouchbaseLite.h"
#include "Listener.h"
#include "util.h"

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

static napi_value replicator_status_to_napi_object(napi_env env, CBLReplicatorStatus replicatorStatus)
{
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

// Lifecycle

// CBLReplicator_Create

typedef struct ReplicationCallbacks
{
  napi_threadsafe_function conflictResolver;
  napi_threadsafe_function pullFilter;
  napi_threadsafe_function pushFilter;
} replication_callbacks;

typedef struct ConflictResolverDocumentInfo
{
  FLString documentID;
  const CBLDocument *localDocument;
  const CBLDocument *remoteDocument;
  const CBLDocument *returnValue;
} conflict_resolver_document_info;

static const CBLDocument *ConflictResolver(void *context, FLString documentID, const CBLDocument *localDocument, const CBLDocument *remoteDocument)
{
  replication_callbacks callbacks = *(replication_callbacks *)context;
  conflict_resolver_document_info *data = malloc(sizeof(documentID) + sizeof(localDocument) + sizeof(remoteDocument) + sizeof(localDocument));
  data->documentID = documentID;
  data->localDocument = localDocument;
  data->remoteDocument = remoteDocument;

  CHECK(napi_acquire_threadsafe_function(callbacks.pullFilter));
  CHECK(napi_call_threadsafe_function(callbacks.pullFilter, data, napi_tsfn_blocking));
  CHECK(napi_release_threadsafe_function(callbacks.pullFilter, napi_tsfn_release));

  return data->returnValue;
}

static void ConflictResolverCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  napi_value args[1];
  conflict_resolver_document_info docInfo = *(conflict_resolver_document_info *)data;

  napi_value res;
  CHECK(napi_create_object(env, &res));

  napi_value documentID;
  CHECK(napi_create_string_utf8(env, docInfo.documentID.buf, docInfo.documentID.size, &documentID));
  CHECK(napi_set_named_property(env, res, "documentID", documentID));

  napi_value localDocument;
  external_document_ref *localDocumentRef = createExternalDocumentRef((CBLDocument *)docInfo.localDocument);
  CHECK(napi_create_external(env, localDocumentRef, finalize_replicator_database_external, NULL, &localDocument));
  CHECK(napi_set_named_property(env, res, "localDocument", localDocument));

  napi_value remoteDocument;
  external_document_ref *remoteDocumentRef = createExternalDocumentRef((CBLDocument *)docInfo.remoteDocument);
  CHECK(napi_create_external(env, remoteDocumentRef, finalize_replicator_database_external, NULL, &remoteDocument));
  CHECK(napi_set_named_property(env, res, "remoteDocument", remoteDocument));

  args[0] = res;

  CHECK(napi_call_function(env, undefined, js_cb, 1, args, NULL));

  free(data);
}

typedef struct ReplicationFilterDocument
{
  CBLDocument *document;
  CBLDocumentFlags flags;
  bool returnValue;
} replication_filter_document;

static bool PullFilter(void *context, CBLDocument *document, CBLDocumentFlags flags)
{
  replication_callbacks callbacks = *(replication_callbacks *)context;
  replication_filter_document *data = malloc(sizeof(document) + sizeof(flags));
  data->document = document;
  data->flags = flags;

  CHECK(napi_acquire_threadsafe_function(callbacks.pullFilter));
  CHECK(napi_call_threadsafe_function(callbacks.pullFilter, data, napi_tsfn_blocking));
  CHECK(napi_release_threadsafe_function(callbacks.pullFilter, napi_tsfn_release));

  return data->returnValue;
}

static bool PushFilter(void *context, CBLDocument *document, CBLDocumentFlags flags)
{
  replication_callbacks callbacks = *(replication_callbacks *)context;
  replication_filter_document *data = malloc(sizeof(document) + sizeof(flags));
  data->document = document;
  data->flags = flags;

  CHECK(napi_acquire_threadsafe_function(callbacks.pushFilter));
  CHECK(napi_call_threadsafe_function(callbacks.pushFilter, data, napi_tsfn_blocking));
  CHECK(napi_release_threadsafe_function(callbacks.pushFilter, napi_tsfn_release));

  return data->returnValue;
}

static void ReplicationFilterCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  napi_value args[1];
  replication_filter_document replicationFilterData = *(replication_filter_document *)data;

  napi_value res;
  CHECK(napi_create_object(env, &res));

  napi_value document;
  external_document_ref *documentRef = createExternalDocumentRef(replicationFilterData.document);
  CHECK(napi_create_external(env, documentRef, finalize_replicator_database_external, NULL, &document));
  CHECK(napi_set_named_property(env, res, "document", document));

  napi_value replicatedDocumentDeleted;
  napi_value replicatedDocumentRemoved;
  CHECK(napi_get_boolean(env, replicationFilterData.flags == kCBLDocumentFlagsDeleted, &replicatedDocumentDeleted));
  CHECK(napi_get_boolean(env, replicationFilterData.flags == kCBLDocumentFlagsAccessRemoved, &replicatedDocumentRemoved));
  CHECK(napi_set_named_property(env, res, "deleted", replicatedDocumentDeleted));
  CHECK(napi_set_named_property(env, res, "accessRemoved", replicatedDocumentRemoved));

  args[0] = res;

  napi_value returnValue;
  CHECK(napi_call_function(env, undefined, js_cb, 1, args, &returnValue));
  CHECK(napi_get_value_bool(env, returnValue, &((replication_filter_document *)data)->returnValue));

  free(data);
}

napi_value
Replicator_Create(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  replication_callbacks *callbacks = malloc(sizeof(replication_callbacks));

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

  bool hasConflictResolver;
  CHECK(napi_has_named_property(env, napiConfig, "conflictResolver", &hasConflictResolver));

  if (hasConflictResolver)
  {
    napi_value conflictResolver;
    CHECK(napi_get_named_property(env, napiConfig, "conflictResolver", &conflictResolver));
    napi_value conflict_resolver_async_resource_name;
    CHECK(napi_create_string_utf8(env,
                                  "couchbase-lite replication conflict resolver",
                                  NAPI_AUTO_LENGTH,
                                  &conflict_resolver_async_resource_name));

    napi_threadsafe_function conflictResolverCallback;
    CHECK(napi_create_threadsafe_function(env, conflictResolver, NULL, conflict_resolver_async_resource_name, 0, 1, NULL, NULL, NULL, ConflictResolverCallJS, &conflictResolverCallback));
    CHECK(napi_unref_threadsafe_function(env, conflictResolverCallback));

    callbacks->conflictResolver = conflictResolverCallback;

    config.conflictResolver = ConflictResolver;
  }

  bool hasPullFilter;
  CHECK(napi_has_named_property(env, napiConfig, "pullFilter", &hasPullFilter));

  if (hasPullFilter)
  {
    napi_value pullFilter;
    CHECK(napi_get_named_property(env, napiConfig, "pullFilter", &pullFilter));

    napi_value pull_filter_async_resource_name;
    CHECK(napi_create_string_utf8(env,
                                  "couchbase-lite replication pull filter",
                                  NAPI_AUTO_LENGTH,
                                  &pull_filter_async_resource_name));

    napi_threadsafe_function pullFilterCallback;
    CHECK(napi_create_threadsafe_function(env, pullFilter, NULL, pull_filter_async_resource_name, 0, 1, NULL, NULL, NULL, ReplicationFilterCallJS, &pullFilterCallback));
    CHECK(napi_unref_threadsafe_function(env, pullFilterCallback));

    callbacks->pullFilter = pullFilterCallback;

    config.pullFilter = PullFilter;
  }

  bool hasPushFilter;
  CHECK(napi_has_named_property(env, napiConfig, "pushFilter", &hasPushFilter));

  if (hasPushFilter)
  {
    napi_value pushFilter;
    CHECK(napi_get_named_property(env, napiConfig, "pushFilter", &pushFilter));

    napi_value push_filter_async_resource_name;
    CHECK(napi_create_string_utf8(env,
                                  "couchbase-lite replication push filter",
                                  NAPI_AUTO_LENGTH,
                                  &push_filter_async_resource_name));

    napi_threadsafe_function pushFilterCallback;
    CHECK(napi_create_threadsafe_function(env, pushFilter, NULL, push_filter_async_resource_name, 0, 1, NULL, NULL, NULL, ReplicationFilterCallJS, &pushFilterCallback));
    CHECK(napi_unref_threadsafe_function(env, pushFilterCallback));

    callbacks->pushFilter = pushFilterCallback;

    config.pushFilter = PushFilter;
  }

  config.context = callbacks;

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
  napi_value res = replicator_status_to_napi_object(env, replicatorStatus);

  return res;
}

// CBLReplicator_IsDocumentPending
napi_value Replicator_IsDocumentPending(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  size_t buffer_size = 128;
  char docID[buffer_size];
  CHECK(napi_get_value_string_utf8(env, args[1], docID, buffer_size, NULL));

  CBLError err;
  bool isPending = CBLReplicator_IsDocumentPending(replicatorRef->replicator, FLStr(docID), &err);

  if (err.code != 0)
  {
    throwCBLError(env, err);
    return NULL;
  }

  napi_value res;
  CHECK(napi_get_boolean(env, isPending, &res));

  return res;
}

// CBLReplicator_PendingDocumentIDs
napi_value Replicator_PendingDocumentIDs(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  CBLError err;
  FLDict docIDs = CBLReplicator_PendingDocumentIDs(replicatorRef->replicator, &err);

  if (!docIDs)
  {
    throwCBLError(env, err);
    return NULL;
  }

  napi_value res;
  FLStringResult docIDsJSON = FLValue_ToJSON((FLValue)docIDs);
  CHECK(napi_create_string_utf8(env, docIDsJSON.buf, docIDsJSON.size, &res))
  FLSliceResult_Release(docIDsJSON);

  return res;
}

// CBLReplicator_AddChangeListener
static void ReplicatorChangeListener(void *cb, CBLReplicator *replicator, const CBLReplicatorStatus *status)
{
  CHECK(napi_acquire_threadsafe_function((napi_threadsafe_function)cb));
  CHECK(napi_call_threadsafe_function((napi_threadsafe_function)cb, (void *)status, napi_tsfn_nonblocking));
  CHECK(napi_release_threadsafe_function((napi_threadsafe_function)cb, napi_tsfn_release));
}

static void ReplicatorChangeListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  napi_value args[1];
  CBLReplicatorStatus status = *(CBLReplicatorStatus *)data;
  napi_value res = replicator_status_to_napi_object(env, status);

  args[0] = res;

  CHECK(napi_call_function(env, undefined, js_cb, 1, args, NULL));

  free(data);
}

napi_value
Replicator_AddChangeListener(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  napi_value async_resource_name;
  CHECK(napi_create_string_utf8(env,
                                "couchbase-lite replication change listener",
                                NAPI_AUTO_LENGTH,
                                &async_resource_name));

  napi_threadsafe_function listenerCallback;
  CHECK(napi_create_threadsafe_function(env, args[1], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, ReplicatorChangeListenerCallJS, &listenerCallback));
  CHECK(napi_unref_threadsafe_function(env, listenerCallback));

  CBLListenerToken *token = CBLReplicator_AddChangeListener(replicatorRef->replicator, ReplicatorChangeListener, listenerCallback);

  if (!token)
  {
    napi_throw_error(env, "", "Error adding replication change listener\n");
  }

  struct StopListenerData *stopListenerData = newStopListenerData(listenerCallback, token);
  napi_value stopListener;
  CHECK(napi_create_function(env, "stopReplicationChangeListener", NAPI_AUTO_LENGTH, StopChangeListener, stopListenerData, &stopListener));

  return stopListener;
}

// CBLReplicator_AddDocumentReplicationListener
typedef struct ReplicatedDocumentStatus
{
  const CBLReplicatedDocument *documents;
  unsigned numDocuments;
  bool isPush;
} replicated_document_status;

static void DocumentReplicationListener(void *cb, CBLReplicator *replicator, bool isPush, unsigned numDocuments, const CBLReplicatedDocument *documents)
{
  replicated_document_status *data = malloc(sizeof(isPush) + sizeof(numDocuments) + sizeof(*documents));
  data->documents = documents;
  data->numDocuments = numDocuments;
  data->isPush = isPush;

  CHECK(napi_acquire_threadsafe_function((napi_threadsafe_function)cb));
  CHECK(napi_call_threadsafe_function((napi_threadsafe_function)cb, data, napi_tsfn_nonblocking));
  CHECK(napi_release_threadsafe_function((napi_threadsafe_function)cb, napi_tsfn_release));
}

static void DocumentReplicationListenerCallJS(napi_env env, napi_value js_cb, void *context, void *data)
{
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));

  napi_value args[1];
  replicated_document_status docStatus = *(replicated_document_status *)data;

  napi_value res;
  CHECK(napi_create_object(env, &res));

  napi_value direction;

  if (docStatus.isPush)
  {
    CHECK(napi_create_string_utf8(env, "push", NAPI_AUTO_LENGTH, &direction));
  }
  else
  {
    CHECK(napi_create_string_utf8(env, "pull", NAPI_AUTO_LENGTH, &direction));
  }

  CHECK(napi_set_named_property(env, res, "direction", direction));

  napi_value documents;
  CHECK(napi_create_array_with_length(env, (size_t)docStatus.numDocuments, &documents));

  for (unsigned int i = 0; i < docStatus.numDocuments; i++)
  {
    napi_value replicatedDocument;
    CHECK(napi_create_object(env, &replicatedDocument));

    napi_value replicatedDocumentID;
    CHECK(napi_create_string_utf8(env, docStatus.documents[i].ID.buf, docStatus.documents[i].ID.size, &replicatedDocumentID));
    CHECK(napi_set_named_property(env, replicatedDocument, "id", replicatedDocumentID));

    napi_value replicatedDocumentDeleted;
    napi_value replicatedDocumentRemoved;
    CHECK(napi_get_boolean(env, docStatus.documents[i].flags == kCBLDocumentFlagsDeleted, &replicatedDocumentDeleted));
    CHECK(napi_get_boolean(env, docStatus.documents[i].flags == kCBLDocumentFlagsAccessRemoved, &replicatedDocumentRemoved));
    CHECK(napi_set_named_property(env, replicatedDocument, "deleted", replicatedDocumentDeleted));
    CHECK(napi_set_named_property(env, replicatedDocument, "accessRemoved", replicatedDocumentRemoved));

    CBLError docError = docStatus.documents[i].error;

    if (docError.code != 0)
    {
      FLSliceResult errorMsg = CBLError_Message(&docError);
      napi_value napiErrorMsg;

      CHECK(napi_create_string_utf8(env, errorMsg.buf, errorMsg.size, &napiErrorMsg))
      CHECK(napi_set_named_property(env, replicatedDocument, "error", napiErrorMsg));
    }

    CHECK(napi_set_element(env, documents, i, replicatedDocument));
  }

  CHECK(napi_set_named_property(env, res, "documents", documents));

  args[0] = res;

  CHECK(napi_call_function(env, undefined, js_cb, 1, args, NULL));

  free(data);
}

napi_value Replicator_AddDocumentReplicationListener(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value args[argc];
  CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  external_replicator_ref *replicatorRef;
  CHECK(napi_get_value_external(env, args[0], (void *)&replicatorRef));

  napi_value async_resource_name;
  CHECK(napi_create_string_utf8(env,
                                "couchbase-lite document replication listener",
                                NAPI_AUTO_LENGTH,
                                &async_resource_name));

  napi_threadsafe_function listenerCallback;
  CHECK(napi_create_threadsafe_function(env, args[1], NULL, async_resource_name, 0, 1, NULL, NULL, NULL, DocumentReplicationListenerCallJS, &listenerCallback));
  CHECK(napi_unref_threadsafe_function(env, listenerCallback));

  CBLListenerToken *token = CBLReplicator_AddDocumentReplicationListener(replicatorRef->replicator, DocumentReplicationListener, listenerCallback);

  if (!token)
  {
    napi_throw_error(env, "", "Error document replication listener\n");
  }

  struct StopListenerData *stopListenerData = newStopListenerData(listenerCallback, token);
  napi_value stopListener;
  CHECK(napi_create_function(env, "stopDocumentReplicationListener", NAPI_AUTO_LENGTH, StopChangeListener, stopListenerData, &stopListener));

  return stopListener;
}
