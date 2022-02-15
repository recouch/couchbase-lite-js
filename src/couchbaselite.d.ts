/* eslint-disable camelcase */

declare module '*couchbaselite.node' {
  interface BlobRef extends Symbol {
    type: 'Blob'
  }

  interface BlobReadStreamRef extends Symbol {
    type: 'BlobReadStream'
  }

  interface BlobWriteStreamRef extends Symbol {
    type: 'BlobWriteStream'
  }

  interface DatabaseRef extends Symbol {
    type: 'Database'
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
  interface DocumentRef<T = unknown> extends Symbol {
    type: 'Document'
    mutable: false
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
  interface MutableDocumentRef<T = unknown> extends Symbol {
    type: 'MutableDocument'
    mutable: true
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
  interface QueryRef<T = unknown, P = Record<string, string>> extends Symbol {
    type: 'Query'
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
  interface ReplicatorRef extends Symbol {
    type: 'Replicator'
  }

  interface BlobMetadata {
    '@type': 'blob'
    content_type?: string
    digest: string
    length: number
  }

  type ReplicatorType = 'pushAndPull' | 'push' | 'pull'

  interface ReplicatorConfiguration {
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

  interface ReplicatorStatus {
    activity: string
    error: string
    progress: {
      complete: number
      documentCount: number
    }
  }

  type DatabaseChangeListener = (docIDs: string[]) => void
  type RemoveDatabaseChangeListener = () => void

  type DocumentChangeListener = (docID: string) => void
  type RemoveDocumentChangeListener = () => void

  type QueryChangeListener = (results: string) => void
  type RemoveQueryChangeListener = () => void

  type ReplicatorChangeListener = (status: ReplicatorStatus) => void
  type RemoveReplicatorChangeListener = () => void

  type DocumentReplicationListener = (changes: {
    direction: 'push' | 'pull'
    documents: {
      id: string
      accessRemoved: boolean
      deleted: boolean
      error?: string
    }[]
  }) => void
  type RemoveDocumentReplicationListener = () => void

  enum QueryLanguage {}

  const CBLJSONLanguage: QueryLanguage
  const CBLN1QLLanguage: QueryLanguage

  function Database_BeginTransaction(database: DatabaseRef): boolean
  function Database_Close(database: DatabaseRef): boolean
  function Database_Delete(database: DatabaseRef): boolean
  function Database_EndTransaction(database: DatabaseRef, commit: boolean): boolean
  function Database_Open(name: string, directory?: string): DatabaseRef
  function Database_Name(database: DatabaseRef): string
  function Database_Path(database: DatabaseRef): string
  function DeleteDatabase(name: string, directory: string): boolean
  function Database_AddChangeListener(database: DatabaseRef, handler: DatabaseChangeListener): RemoveDatabaseChangeListener

  function Database_AddDocumentChangeListener(database: DatabaseRef, docID: string, handler: DocumentChangeListener): RemoveDocumentChangeListener
  function Database_GetDocument<T = unknown>(database: DatabaseRef, id: string): DocumentRef<T> | null
  function Database_GetMutableDocument<T = unknown>(database: DatabaseRef, id: string): MutableDocumentRef<T> | null
  function Database_SaveDocument(database: DatabaseRef, doc: MutableDocumentRef): boolean
  function Database_DeleteDocument(database: DatabaseRef, doc: DocumentRef | MutableDocumentRef): boolean
  function Document_Create<T = unknown>(): MutableDocumentRef<T>
  function Document_CreateWithID<T = unknown>(id: string): MutableDocumentRef<T>
  function Document_CreateJSON<T = unknown>(doc: DocumentRef<T> | MutableDocumentRef<T>): string
  function Document_ID(doc: DocumentRef | MutableDocumentRef): string
  function Document_SetJSON<T = unknown>(doc: MutableDocumentRef<T>, value: string): boolean

  function Database_CreateQuery<T = unknown, P = Record<string, string>>(database: DatabaseRef, queryLanguage: QueryLanguage, query: string): QueryRef<T, P>
  function Query_AddChangeListener<T = unknown>(query: QueryRef<T>, handler: QueryChangeListener): RemoveQueryChangeListener
  function Query_Execute<T = unknown>(query: QueryRef<T>): string
  function Query_Explain(query: QueryRef): string
  function Query_Parameters(query: QueryRef): string
  function Query_SetParameters(query: QueryRef, parametersJSON: string): boolean

  function Replicator_AddChangeListener(replicator: ReplicatorRef, handler: ReplicatorChangeListener): RemoveReplicatorChangeListener
  function Replicator_AddDocumentReplicationListener(replicator: ReplicatorRef, handler: DocumentReplicationListener): RemoveDocumentReplicationListener
  function Replicator_Create(config: ReplicatorConfiguration): ReplicatorRef
  function Replicator_Config(replicator: ReplicatorRef): ReplicatorConfiguration
  function Replicator_IsDocumentPending(replicator: ReplicatorRef, documentID: string): boolean
  function Replicator_PendingDocumentIDs(replicator: ReplicatorRef): string[]
  function Replicator_Start(replicator: ReplicatorRef, resetCheckpoint?: boolean): boolean
  function Replicator_Status(replicator: ReplicatorRef): ReplicatorStatus
  function Replicator_Stop(replicator: ReplicatorRef): boolean

  function Blob_Content(blob: BlobRef): Buffer
  function Blob_ContentType(blob: BlobRef): string
  function Blob_CreateJSON(blob: BlobRef): string
  function Blob_CreateWithData(contentType: string, buffer: Buffer): BlobRef
  function Blob_CreateWithStream(contentType: string, stream: BlobWriteStreamRef): BlobRef
  function Blob_Digest(blob: BlobRef): string
  function Blob_Equals(blob: BlobRef, anotherBlob: BlobRef): boolean
  function Blob_Length(blob: BlobRef): string
  function Blob_OpenContentStream(blob: BlobRef): BlobReadStreamRef
  function BlobReader_Close(stream: BlobReadStreamRef): void
  function BlobReader_Read(stream: BlobReadStreamRef, maxLength: number): Buffer
  function BlobWriter_Close(stream: BlobWriteStreamRef): void
  function BlobWriter_Create(database: DatabaseRef): BlobWriteStreamRef
  function BlobWriter_Write(stream: BlobWriteStreamRef, buffer: Buffer): boolean
  function Database_GetBlob(database: DatabaseRef, properties: string): BlobRef
  function Database_SaveBlob(database: DatabaseRef, blob: BlobRef): boolean
  function Document_GetBlob(doc: DocumentRef | MutableDocumentRef, property: string): BlobRef
  function Document_IsBlob(doc: DocumentRef | MutableDocumentRef, property: string): boolean
  function Document_SetBlob(doc: MutableDocumentRef, property: string, blob: BlobRef): void
}
