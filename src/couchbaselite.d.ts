/* eslint-disable camelcase */

declare module '*couchbaselite.node' {
  import { BlobMetadata, BlobReadStreamRef, BlobRef, BlobWriteStreamRef, DatabaseChangeListener, DatabaseRef, DocumentChangeListener, DocumentRef, DocumentReplicationListener, MutableDocumentRef, QueryLanguage, QueryRef, RemoveDatabaseChangeListener, RemoveDocumentChangeListener, RemoveDocumentReplicationListener, RemoveQueryChangeListener, RemoveReplicatorChangeListener, ReplicatorChangeListener, ReplicatorConfiguration, ReplicatorRef, ReplicatorStatus } from 'src/types'

  type QueryChangeListener<T> = (results: T[]) => void

  const cblite: {
    readonly CBLJSONLanguage: QueryLanguage
    readonly CBLN1QLLanguage: QueryLanguage

    /**
     * Add a listener to database change events.
     * @param database the {@link @recouch/couchbase-lite#DatabaseRef} to listen to.
     * @param handler event handler with `(docIDs: string[]) => void`
     */
    addDatabaseChangeListener(database: DatabaseRef, handler: DatabaseChangeListener): RemoveDatabaseChangeListener

    /**
     * Begin a transaction.
     * @param database {@link @recouch/couchbase-lite#DatabaseRef}.
     */
    beginTransaction(database: DatabaseRef): boolean
    /**
     * Close the database.
     * @param database {@link @recouch/couchbase-lite#DatabaseRef}.
     */
    closeDatabase(database: DatabaseRef): boolean

    deleteDatabase(name: string, directory: string): boolean
    deleteDatabase(database: DatabaseRef): boolean
    endTransaction(database: DatabaseRef, commit: boolean): boolean
    openDatabase(name: string, directory?: string): DatabaseRef
    databaseName(database: DatabaseRef): string
    databasePath(database: DatabaseRef): string

    addDocumentChangeListener(database: DatabaseRef, docID: string, handler: DocumentChangeListener): RemoveDocumentChangeListener
    deleteDocument(database: DatabaseRef, doc: DocumentRef | MutableDocumentRef): boolean
    getDocument<T = unknown>(database: DatabaseRef, id: string): DocumentRef<T> | null
    getMutableDocument<T = unknown>(database: DatabaseRef, id: string): MutableDocumentRef<T> | null
    saveDocument(database: DatabaseRef, doc: MutableDocumentRef): boolean
    createDocument<T = unknown>(id?: string): MutableDocumentRef<T>
    getDocumentJSON<T = unknown>(doc: DocumentRef<T> | MutableDocumentRef<T>): string
    getDocumentID(doc: DocumentRef | MutableDocumentRef): string
    getDocumentProperties<T = unknown>(doc: DocumentRef<T> | MutableDocumentRef<T>): T
    setDocumentJSON<T = unknown>(doc: MutableDocumentRef<T>, value: string): boolean
    setDocumentProperties<T = unknown>(doc: MutableDocumentRef<T>, value: T): boolean

    createQuery<T = unknown, P = Record<string, string>>(database: DatabaseRef, query: string): QueryRef<T, P>
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    createQuery<T = unknown, P = Record<string, string>>(database: DatabaseRef, query: any[]): QueryRef<T, P>
    createQuery<T = unknown, P = Record<string, string>>(database: DatabaseRef, queryLanguage: QueryLanguage, query: string): QueryRef<T, P>
    addQueryChangeListener<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>, handler: QueryChangeListener<T>): RemoveQueryChangeListener
    executeQuery<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>): T[]
    explainQuery<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>): string
    getQueryParameters<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>): Partial<P>
    setQueryParameters<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>, parametersJSON: Partial<P>): void

    addDocumentReplicationListener(replicator: ReplicatorRef, handler: DocumentReplicationListener): RemoveDocumentReplicationListener
    addReplicatorChangeListener(replicator: ReplicatorRef, handler: ReplicatorChangeListener): RemoveReplicatorChangeListener
    createReplicator(config: ReplicatorConfiguration): ReplicatorRef
    documentsPendingReplication(replicator: ReplicatorRef): string[]
    isDocumentPendingReplication(replicator: ReplicatorRef, documentID: string): boolean
    replicatorConfiguration(replicator: ReplicatorRef): ReplicatorConfiguration
    replicatorStatus(replicator: ReplicatorRef): ReplicatorStatus
    setHostReachable(replicator: ReplicatorRef, reachable: boolean): boolean
    startReplicator(replicator: ReplicatorRef, resetCheckpoint?: boolean): boolean
    stopReplicator(replicator: ReplicatorRef): boolean

    blobContent(blob: BlobRef): Buffer
    blobContentType(blob: BlobRef): string
    blobCreateJson(blob: BlobRef): string
    blobProperties(blob: BlobRef): BlobMetadata
    createBlobWithData(contentType: string, buffer: Buffer): BlobRef
    createBlobWithStream(contentType: string, stream: BlobWriteStreamRef): BlobRef
    blobDigest(blob: BlobRef): string
    blobEquals(blob: BlobRef, anotherBlob: BlobRef): boolean
    blobLength(blob: BlobRef): number
    openBlobContentStream(blob: BlobRef): BlobReadStreamRef
    closeBlobReader(stream: BlobReadStreamRef): void
    readBlobReader(stream: BlobReadStreamRef, maxLength: number): Buffer
    closeBlobWriter(stream: BlobWriteStreamRef): void
    createBlobWriter(database: DatabaseRef): BlobWriteStreamRef
    writeBlobWriter(stream: BlobWriteStreamRef, buffer: Buffer): boolean
    databaseGetBlob(database: DatabaseRef, properties: BlobMetadata): BlobRef
    databaseSaveBlob(database: DatabaseRef, blob: BlobRef): boolean
    documentGetBlob(doc: DocumentRef | MutableDocumentRef, property: string): BlobRef
    documentIsBlob(doc: DocumentRef | MutableDocumentRef, property: string): boolean
    documentSetBlob(doc: MutableDocumentRef, property: string, blob: BlobRef): void
  }

  export = cblite
}
