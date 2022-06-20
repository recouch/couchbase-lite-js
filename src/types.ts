
export interface BlobRef extends Symbol {
  type: 'Blob'
}

export interface BlobReadStreamRef extends Symbol {
  type: 'BlobReadStream'
}

export interface BlobWriteStreamRef extends Symbol {
  type: 'BlobWriteStream'
}

/**
 * Opaque reference to a database, as returned by {@link https://docs.couchbase.com/mobile/3.0.0-beta02/couchbase-lite-c/C/html/group__database.html#ga7c119edb784a630c2de21806db4d4e4d}.
 */
export interface DatabaseRef extends Symbol {
  type: 'Database'
}

export interface DocumentRef<T = unknown> extends Symbol {
  __: T
  type: 'Document'
  mutable: false
}

export interface MutableDocumentRef<T = unknown> extends Symbol {
  __: T
  type: 'MutableDocument'
  mutable: true
}

export interface QueryRef<T = unknown, P = Record<string, string>> extends Symbol {
  __: T
  ___: P
  type: 'Query'
}

export interface ReplicatorRef extends Symbol {
  type: 'Replicator'
}

export interface BlobMetadata {
  '@type': 'blob'
  // eslint-disable-next-line camelcase
  content_type?: string
  digest: string
  length: number
}

export type ReplicatorType = 'pushAndPull' | 'push' | 'pull'

export interface ReplicatorConfiguration {
  database: DatabaseRef
  endpoint: DatabaseRef | string
  replicatorType?: ReplicatorType
  continuous?: boolean
  disableAutoPurge?: boolean
  maxAttempts?: number
  maxAttemptWaitTime?: number
  heartbeat?: number,
  conflictResolver?: (document: { documentID: string; localDocument: DocumentRef; remoteDocument: DocumentRef }) => void
  pushFilter?: (document: { document: DocumentRef; accessRemoved: boolean; deleted: boolean }) => void
  pullFilter?: (document: { document: DocumentRef; accessRemoved: boolean; deleted: boolean }) => void
}

export interface ReplicatorStatus {
  activity: 'stopped' | 'offline' | 'connecting' | 'idle' | 'busy'
  error: string
  progress: {
    complete: number
    documentCount: number
  }
}

export type DatabaseChangeListener = (docIDs: string[]) => void
export type RemoveDatabaseChangeListener = () => void

export type DocumentChangeListener = (docID: string) => void
export type RemoveDocumentChangeListener = () => void

export type QueryChangeListener<T> = (results: T[]) => void
export type RemoveQueryChangeListener = () => void

export type ReplicatorChangeListener = (status: ReplicatorStatus) => void
export type RemoveReplicatorChangeListener = () => void

export type ReplicatedDocInfo = {
  id: string
  accessRemoved: boolean
  deleted: boolean
  error?: string
}

export type DocumentReplicationListener = (
  direction: 'push' | 'pull',
  documents: ReplicatedDocInfo[]
) => void
export type RemoveDocumentReplicationListener = () => void

export enum QueryLanguage {}
