import { BlobMetadata, BlobReadStreamRef, BlobRef, BlobWriteStreamRef, DatabaseChangeListener, DatabaseRef, DocumentChangeListener, DocumentRef, DocumentReplicationListener, MutableDocumentRef, QueryChangeListener, QueryRef, RemoveDatabaseChangeListener, RemoveDocumentChangeListener, RemoveDocumentReplicationListener, RemoveQueryChangeListener, RemoveReplicatorChangeListener, ReplicatorChangeListener, ReplicatorConfiguration, ReplicatorRef, ReplicatorStatus } from '../types'
import { blobContent, blobContentType, blobCreateJson, blobProperties, blobDigest, blobEquals, blobLength, openBlobContentStream, closeBlobReader, readBlobReader, writeBlobWriter, closeBlobWriter, createBlobWriter, databaseGetBlob, databaseSaveBlob, createBlobWithStream, documentGetBlob, documentIsBlob, documentSetBlob } from './Blob'
import { abortTransaction, addDatabaseChangeListener, beginTransaction, closeDatabase, commitTransaction, databaseName, databasePath, deleteDatabase, endTransaction } from './Database'
import { addDocumentChangeListener, createDocument, deleteDocument, getDocument, getDocumentID, getDocumentProperties, getMutableDocument, saveDocument, setDocumentProperties } from './Document'
import { addQueryChangeListener, createQuery, executeQuery, explainQuery, getQueryParameters, setQueryParameters } from './Query'
import { addReplicatorChangeListener, addDocumentReplicationListener, replicatorConfiguration, isDocumentPendingReplication, documentsPendingReplication, startReplicator, replicatorStatus, stopReplicator, createReplicator } from './Replicator'

export interface ScopedBlobReadStream {
  close: () => void
  read: (maxLength: number) => Buffer
}

export interface ScopedBlob{
  content: () => Buffer
  contentType: () => string
  createJson: () => string
  properties: () => BlobMetadata
  digest: () => string
  equals: (anotherBlob: BlobRef) => boolean
  length: () => number
  openContentStream: () => ScopedBlobReadStream
}

// Start interface definition here to allow ScopedDocuments to reference it
// eslint-disable-next-line @typescript-eslint/no-empty-interface, import/export
export interface ScopedDatabase {}

export interface ScopedDocument<T = unknown> {
  database: () => ScopedDatabase
  delete: () => boolean
  getID: () => string
  getProperties: () => T
  getBlob: (property: string) => ScopedBlob
  isBlob: (property: string) => boolean
}

export interface ScopedMutableDocument<T = unknown> extends ScopedDocument<T> {
  save: () => boolean
  setProperties: (value: T) => boolean
  setBlob: (property: string, blob: BlobRef) => void
}

export interface ScopedQuery<T = unknown[], P = Record<string, string>> {
  addChangeListener: (handler: QueryChangeListener<T>) => RemoveQueryChangeListener
  execute: () => T[]
  explain: () => string
  getParameters: () => Partial<P>
  setParameters: (parameters: Partial<P>) => boolean
}

export interface ScopedBlobWriteStream {
  close: () => void
  createBlob: (contentType: string) => BlobRef
  write: (buffer: Buffer) => boolean
}

export interface ScopedReplicator {
  addChangeListener: (handler: ReplicatorChangeListener) => RemoveReplicatorChangeListener
  addDocumentReplicationListener: (handler: DocumentReplicationListener) => RemoveDocumentReplicationListener
  configuration: () => ReplicatorConfiguration
  documentsPendingReplication: () =>string[]
  isDocumentPendingReplication: (documentID: string) =>boolean
  start: (resetCheckpoint?: boolean)=>boolean
  status: ()=> ReplicatorStatus
  stop: () => boolean
}

// eslint-disable-next-line import/export, no-redeclare
export interface ScopedDatabase {
  abortTransaction: () => boolean
  addChangeListener: (handler: DatabaseChangeListener) => RemoveDatabaseChangeListener
  beginTransaction: () => boolean
  close: () => boolean;
  commitTransaction: () => boolean
  name: () => string
  path: () => string
  delete: () => boolean
  endTransaction: (commit: boolean) => boolean

  // Blob methods
  createBlobWriter: () => ScopedBlobWriteStream,
  getBlob: (properties: BlobMetadata) => ScopedBlob,
  saveBlob: (blob: BlobRef) => boolean

  // Document methods
  addDocumentChangeListener: (docID: string, handler: DocumentChangeListener) => RemoveDocumentChangeListener
  createDocument: <T = unknown>(id?: string) => ScopedMutableDocument<T>
  getDocument: <T = unknown>(id:string) => ScopedDocument<T>|null
  getMutableDocument: <T = unknown>(id: string) => ScopedMutableDocument<T>|null

  // Query methods
  createQuery: <T = unknown[], P = Record<string, string>>(query: string | unknown[]) => ScopedQuery<T, P>,

  // Replicator methods
  createReplicator: (config: Omit<ReplicatorConfiguration, 'database'>) => ScopedReplicator
}

export const scopeBlob = (blobRef: BlobRef): ScopedBlob => ({
  content: blobContent.bind(null, blobRef),
  contentType: blobContentType.bind(null, blobRef),
  createJson: blobCreateJson.bind(null, blobRef),
  properties: blobProperties.bind(null, blobRef),
  digest: blobDigest.bind(null, blobRef),
  equals: blobEquals.bind(null, blobRef),
  length: blobLength.bind(null, blobRef),
  openContentStream: () => scopeBlobReadStream(openBlobContentStream(blobRef))
})

export const scopeBlobReadStream = (streamRef: BlobReadStreamRef): ScopedBlobReadStream => ({
  close: closeBlobReader.bind(null, streamRef),
  read: readBlobReader.bind(null, streamRef)
})

export const scopeBlobWriteStream = (streamRef: BlobWriteStreamRef) => ({
  close: closeBlobWriter.bind(null, streamRef),
  createBlob: (contentType: string) => createBlobWithStream(contentType, streamRef),
  write: writeBlobWriter.bind(null, streamRef)
})

export const scopeDatabase = (dbRef: DatabaseRef): ScopedDatabase => ({
  // Database methods
  abortTransaction: abortTransaction.bind(null, dbRef),
  addChangeListener: addDatabaseChangeListener.bind(null, dbRef),
  beginTransaction: beginTransaction.bind(null, dbRef),
  close: closeDatabase.bind(null, dbRef),
  commitTransaction: commitTransaction.bind(null, dbRef),
  name: databaseName.bind(null, dbRef),
  path: databasePath.bind(null, dbRef),
  delete: deleteDatabase.bind(null, dbRef),
  endTransaction: endTransaction.bind(null, dbRef),

  // Blob methods
  createBlobWriter: () => scopeBlobWriteStream(createBlobWriter(dbRef)),
  getBlob: (properties: BlobMetadata) => scopeBlob(databaseGetBlob(dbRef, properties)),
  saveBlob: databaseSaveBlob.bind(null, dbRef),

  // Document methods
  addDocumentChangeListener: addDocumentChangeListener.bind(null, dbRef),
  createDocument: <T = unknown>(id?: string) => scopeMutableDocument<T>(dbRef, createDocument(id))!,
  getDocument: <T = unknown>(id:string) => scopeDocument<T>(dbRef, getDocument(dbRef, id)),
  getMutableDocument: <T = unknown>(id: string) => scopeMutableDocument<T>(dbRef, getMutableDocument(dbRef, id)),

  // Query methods
  createQuery: <T = unknown[], P = Record<string, string>>(query: string | unknown[]) => scopeQuery(createQuery<T, P>(dbRef, query)),

  // Replicator methods
  createReplicator: (config: Omit<ReplicatorConfiguration, 'database'>) => scopeReplicator(createReplicator({ ...config, database: dbRef }))
})

export const scopeDocument = <T = unknown>(dbRef: DatabaseRef, docRef: DocumentRef<T> | null): ScopedDocument<T> | null => docRef && {
  database: () => scopeDatabase(dbRef),

  delete: deleteDocument.bind(null, dbRef, docRef),
  getID: getDocumentID.bind(null, docRef),
  getProperties: () => getDocumentProperties(docRef),

  // Blob methods
  getBlob: (property: string) => scopeBlob(documentGetBlob(docRef, property)),
  isBlob: documentIsBlob.bind(null, docRef)
}

export const scopeMutableDocument = <T = unknown>(dbRef: DatabaseRef, docRef: MutableDocumentRef<T> | null): ScopedMutableDocument<T> | null => docRef && {
  database: () => scopeDatabase(dbRef),

  delete: deleteDocument.bind(null, dbRef, docRef),
  getID: getDocumentID.bind(null, docRef),
  getProperties: () => getDocumentProperties(docRef),
  save: saveDocument.bind(null, dbRef, docRef),
  setProperties: setDocumentProperties.bind(null, docRef),

  // Blob methods
  getBlob: (property: string) => scopeBlob(documentGetBlob(docRef, property)),
  isBlob: documentIsBlob.bind(null, docRef),
  setBlob: documentSetBlob.bind(null, docRef)
}

export const scopeQuery = <T = unknown[], P = Record<string, string>>(queryRef: QueryRef<T, P>): ScopedQuery<T, P> => ({
  addChangeListener: (handler: QueryChangeListener<T>) => addQueryChangeListener(queryRef, handler),
  execute: () => executeQuery(queryRef),
  explain: explainQuery.bind(null, queryRef),
  getParameters: () => getQueryParameters(queryRef),
  setParameters: setQueryParameters.bind(null, queryRef)
})

export const scopeReplicator = (replicatorRef: ReplicatorRef): ScopedReplicator => ({
  addChangeListener: addReplicatorChangeListener.bind(null, replicatorRef),
  addDocumentReplicationListener: addDocumentReplicationListener.bind(null, replicatorRef),
  configuration: replicatorConfiguration.bind(null, replicatorRef),
  documentsPendingReplication: documentsPendingReplication.bind(null, replicatorRef),
  isDocumentPendingReplication: isDocumentPendingReplication.bind(null, replicatorRef),
  start: startReplicator.bind(null, replicatorRef),
  status: replicatorStatus.bind(null, replicatorRef),
  stop: stopReplicator.bind(null, replicatorRef)
})
