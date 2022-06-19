import { CBL } from '../CBL'
import { DocumentReplicationListener, RemoveDocumentReplicationListener, RemoveReplicatorChangeListener, ReplicatorChangeListener, ReplicatorConfiguration, ReplicatorRef, ReplicatorStatus } from '../types'

export function addDocumentReplicationListener(replicator: ReplicatorRef, handler: DocumentReplicationListener): RemoveDocumentReplicationListener {
  return CBL.Replicator_AddDocumentReplicationListener(replicator, handler)
}

export function addReplicatorChangeListener(replicator: ReplicatorRef, handler: ReplicatorChangeListener): RemoveReplicatorChangeListener {
  return CBL.Replicator_AddChangeListener(replicator, handler)
}

export function createReplicator(config: ReplicatorConfiguration): ReplicatorRef {
  return CBL.Replicator_Create(config)
}

export function documentsPendingReplication(replicator: ReplicatorRef): string[] {
  return CBL.Replicator_PendingDocumentIDs(replicator)
}

export function isDocumentPendingReplication(replicator: ReplicatorRef, documentID: string): boolean {
  return CBL.Replicator_IsDocumentPending(replicator, documentID)
}

export function replicatorConfiguration(replicator: ReplicatorRef): ReplicatorConfiguration {
  return CBL.Replicator_Config(replicator)
}

export function setHostReachable(replicator: ReplicatorRef, reachable = true): boolean {
  return CBL.Replicator_SetHostReachable(replicator, reachable)
}

export function startReplicator(replicator: ReplicatorRef, resetCheckpoint = false): boolean {
  return CBL.Replicator_Start(replicator, resetCheckpoint)
}

export function replicatorStatus(replicator: ReplicatorRef): ReplicatorStatus {
  return CBL.Replicator_Status(replicator)
}

export function stopReplicator(replicator: ReplicatorRef): boolean {
  return CBL.Replicator_Stop(replicator)
}
