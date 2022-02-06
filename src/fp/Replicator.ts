import { CBL } from '../CBL'

export function createReplicator(config: CBL.ReplicatorConfiguration):CBL.ReplicatorRef {
  return CBL.Replicator_Create(config)
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
