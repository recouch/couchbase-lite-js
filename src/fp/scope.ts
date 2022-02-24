import { CBL } from '../CBL'
import { blobContent, blobContentType, blobCreateJson, blobProperties, blobDigest, blobEquals, blobLength, openBlobContentStream, closeBlobReader, readBlobReader, writeBlobWriter, closeBlobWriter, createBlobWriter, databaseGetBlob, databaseSaveBlob, createBlobWithStream, documentGetBlob, documentIsBlob, documentSetBlob } from './Blob'
import { abortTransaction, addDatabaseChangeListener, beginTransaction, closeDatabase, commitTransaction, databaseName, databasePath, deleteDatabase, endTransaction } from './Database'
import { addDocumentChangeListener, createDocument, deleteDocument, getDocument, getDocumentID, getDocumentProperties, getMutableDocument, saveDocument, setDocumentProperties } from './Document'
import { addQueryChangeListener, createQuery, executeQuery, explainQuery, getQueryParameters, QueryChangeListener, setQueryParameters } from './Query'
import { addReplicatorChangeListener, addDocumentReplicationListener, replicatorConfiguration, isDocumentPendingReplication, documentsPendingReplication, startReplicator, replicatorStatus, stopReplicator, createReplicator } from './Replicator'

export interface ScopedBlobReadStream {
  close: () => void
  read: (maxLength: number) => Buffer
}

export interface ScopedBlob{
  content: () => Buffer
  contentType: () => string
  createJson: () => string
  properties: () => CBL.BlobMetadata
  digest: () => string
  equals: (anotherBlob: CBL.BlobRef) => boolean
  length: () => number
  openContentStream: () => ScopedBlobReadStream
}

export interface ScopedDocument<T = unknown> {
  delete: () => boolean
  getID: () => string
  getProperties: () => T
  getBlob: (property: string) => ScopedBlob
  isBlob: (property: string) => boolean
}

export interface ScopedMutableDocument<T = unknown> {
  delete: () => boolean
  getID: () => string
  getProperties: () => T
  save: () => boolean
  setProperties: (value: T) => boolean
  getBlob: (property: string) => ScopedBlob
  isBlob: (property: string) => boolean
  setBlob: (property: string, blob: CBL.BlobRef) => void
}

export interface ScopedQuery<T = unknown[], P = Record<string, string>> {
  addChangeListener: (handler: QueryChangeListener<T>) => CBL.RemoveQueryChangeListener
  execute: () => T[]
  explain: () => string
  getParameters: () => P
  setParameters: (parameters: Partial<P>) => boolean
}

export const scopeBlob = (blobRef: CBL.BlobRef): ScopedBlob => ({
  content: blobContent.bind(null, blobRef),
  contentType: blobContentType.bind(null, blobRef),
  createJson: blobCreateJson.bind(null, blobRef),
  properties: blobProperties.bind(null, blobRef),
  digest: blobDigest.bind(null, blobRef),
  equals: blobEquals.bind(null, blobRef),
  length: blobLength.bind(null, blobRef),
  openContentStream: () => scopeBlobReadStream(openBlobContentStream(blobRef))
})

export const scopeBlobReadStream = (streamRef: CBL.BlobReadStreamRef): ScopedBlobReadStream => ({
  close: closeBlobReader.bind(null, streamRef),
  read: readBlobReader.bind(null, streamRef)
})

export const scopeBlobWriteStream = (streamRef: CBL.BlobWriteStreamRef) => ({
  close: closeBlobWriter.bind(null, streamRef),
  createBlob: (contentType: string) => createBlobWithStream(contentType, streamRef),
  write: writeBlobWriter.bind(null, streamRef)
})

export const scopeDatabase = (dbRef: CBL.DatabaseRef) => ({
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
  getBlob: (properties: CBL.BlobMetadata) => scopeBlob(databaseGetBlob(dbRef, properties)),
  saveBlob: databaseSaveBlob.bind(null, dbRef),

  // Document methods
  addDocumentChangeListener: addDocumentChangeListener.bind(null, dbRef),
  createDocument: <T = unknown>(id?: string) => scopeMutableDocument<T>(dbRef, createDocument(id))!,
  getDocument: <T = unknown>(id:string) => scopeDocument<T>(dbRef, getDocument(dbRef, id)),
  getMutableDocument: <T = unknown>(id: string) => scopeMutableDocument<T>(dbRef, getMutableDocument(dbRef, id)),

  // Query methods
  createQuery: <T = unknown[], P = Record<string, string>>(query: string | unknown[]) => scopeQuery(createQuery<T, P>(dbRef, query)),

  // Replicator methods
  createReplicator: (config: Omit<CBL.ReplicatorConfiguration, 'database'>) => scopeReplicator(createReplicator({ ...config, database: dbRef }))
})

export const scopeDocument = <T = unknown>(dbRef: CBL.DatabaseRef, docRef: CBL.DocumentRef<T> | null): ScopedDocument<T> | null => docRef && {
  delete: deleteDocument.bind(null, dbRef, docRef),
  getID: getDocumentID.bind(null, docRef),
  getProperties: () => getDocumentProperties(docRef),

  // Blob methods
  getBlob: (property: string) => scopeBlob(documentGetBlob(docRef, property)),
  isBlob: documentIsBlob.bind(null, docRef)
}

export const scopeMutableDocument = <T = unknown>(dbRef: CBL.DatabaseRef, docRef: CBL.MutableDocumentRef<T> | null): ScopedMutableDocument<T> | null => docRef && {
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

export const scopeQuery = <T = unknown[], P = Record<string, string>>(queryRef: CBL.QueryRef<T, P>): ScopedQuery<T, P> => ({
  addChangeListener: (handler: QueryChangeListener<T>) => addQueryChangeListener(queryRef, handler),
  execute: () => executeQuery(queryRef),
  explain: explainQuery.bind(null, queryRef),
  getParameters: () => getQueryParameters(queryRef),
  setParameters: setQueryParameters.bind(null, queryRef)
})

export const scopeReplicator = (replicatorRef: CBL.ReplicatorRef) => ({
  addChangeListener: addReplicatorChangeListener.bind(null, replicatorRef),
  addDocumentReplicationListener: addDocumentReplicationListener.bind(null, replicatorRef),
  configuration: replicatorConfiguration.bind(null, replicatorRef),
  documentsPendingReplication: documentsPendingReplication.bind(null, replicatorRef),
  isDocumentPendingReplication: isDocumentPendingReplication.bind(null, replicatorRef),
  start: startReplicator.bind(null, replicatorRef),
  status: replicatorStatus.bind(null, replicatorRef),
  stop: stopReplicator.bind(null, replicatorRef)
})

export type ScopedDatabase = ReturnType<typeof scopeDatabase>
export type ScopedReplicator = ReturnType<typeof scopeReplicator>
