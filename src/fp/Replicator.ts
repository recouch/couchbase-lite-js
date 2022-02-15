import { CBL } from '../CBL'

export function addReplicatorChangeListener(replicator: CBL.ReplicatorRef, handler: CBL.ReplicatorChangeListener): CBL.RemoveReplicatorChangeListener {
  return CBL.Replicator_AddChangeListener(replicator, handler)
}

export function addDocumentReplicationListener(replicator: CBL.ReplicatorRef, handler: CBL.DocumentReplicationListener): CBL.RemoveDocumentReplicationListener {
  return CBL.Replicator_AddDocumentReplicationListener(replicator, handler)
}

export function createReplicator(config: CBL.ReplicatorConfiguration):CBL.ReplicatorRef {
  return CBL.Replicator_Create(config)
}

export function documentsPendingReplication(replicator: CBL.ReplicatorRef): string[] {
  return CBL.Replicator_PendingDocumentIDs(replicator)
}

export function isDocumentPendingReplication(replicator: CBL.ReplicatorRef, documentID: string): boolean {
  return CBL.Replicator_IsDocumentPending(replicator, documentID)
}

export function replicatorConfiguration(replicator: CBL.ReplicatorRef): CBL.ReplicatorConfiguration {
  return CBL.Replicator_Config(replicator)
}

export function startReplicator(replicator: CBL.ReplicatorRef, resetCheckpoint = false): boolean {
  return CBL.Replicator_Start(replicator, resetCheckpoint)
}

export function replicatorStatus(replicator: CBL.ReplicatorRef): CBL.ReplicatorStatus {
  return CBL.Replicator_Status(replicator)
}

export function stopReplicator(replicator: CBL.ReplicatorRef): boolean {
  return CBL.Replicator_Stop(replicator)
}
