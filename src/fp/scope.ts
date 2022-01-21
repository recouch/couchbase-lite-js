import { addDatabaseChangeListener, beginTransaction, CBLDatabaseRef, closeDatabase, databaseName, databasePath, deleteDatabase, endTransaction } from './Database'
import { addDocumentChangeListener, CBLDocumentRef, CBLMutableDocumentRef, createDocument, deleteDocument, getDocument, getDocumentID, getDocumentProperties, getMutableDocument, saveDocument, setDocumentProperties } from './Document'
import { addQueryChangeListener, CBLQueryRef, createQuery, executeQuery, explainQuery, getQueryParameters, QueryChangeListener, RemoveQueryChangeListener, setQueryParameters } from './Query'

export interface ScopedDocument<T = unknown> {
  delete: () => boolean
  getID: () => string
  getProperties: () => T
}

export interface ScopedMutableDocument<T = unknown> {
  delete: () => boolean
  getID: () => string
  getProperties: () => T
  save: () => boolean
  setProperties: (value: T) => boolean
}

export interface ScopedQuery<T = unknown[], P = Record<string, string>> {
  addChangeListener: (handler: QueryChangeListener<T>) => RemoveQueryChangeListener
  execute: () => T[]
  explain: () => string
  getParameters: () => P
  setParameters: (parameters: Partial<P>) => boolean
}

export const scopeDatabase = (dbRef: CBLDatabaseRef) => ({
  // Database methods
  addChangeListener: addDatabaseChangeListener.bind(null, dbRef),
  beginTransaction: beginTransaction.bind(null, dbRef),
  close: closeDatabase.bind(null, dbRef),
  name: databaseName.bind(null, dbRef),
  path: databasePath.bind(null, dbRef),
  delete: deleteDatabase.bind(null, dbRef),
  endTransaction: endTransaction.bind(null, dbRef),

  // Document methods
  addDocumentChangeListener: addDocumentChangeListener.bind(null, dbRef),
  createDocument: <T = unknown>(id?: string) => scopeMutableDocument<T>(dbRef, createDocument(id))!,
  getDocument: <T = unknown>(id:string) => scopeDocument<T>(dbRef, getDocument(dbRef, id)),
  getMutableDocument: <T = unknown>(id: string) => scopeMutableDocument<T>(dbRef, getMutableDocument(dbRef, id)),

  // Query methods
  createQuery: <T = unknown[], P = Record<string, string>>(query: string | unknown[]) => scopeQuery(createQuery<T, P>(dbRef, query))
})

export const scopeDocument = <T = unknown>(dbRef: CBLDatabaseRef, docRef: CBLDocumentRef<T> | null): ScopedDocument<T> | null => docRef && {
  delete: deleteDocument.bind(null, dbRef, docRef),
  getID: getDocumentID.bind(null, docRef),
  getProperties: () => getDocumentProperties(docRef)
}

export const scopeMutableDocument = <T = unknown>(dbRef: CBLDatabaseRef, docRef: CBLMutableDocumentRef<T> | null): ScopedMutableDocument<T> | null => docRef && {
  delete: deleteDocument.bind(null, dbRef, docRef),
  getID: getDocumentID.bind(null, docRef),
  getProperties: () => getDocumentProperties(docRef),
  save: saveDocument.bind(null, dbRef, docRef),
  setProperties: setDocumentProperties.bind(null, docRef)
}

export const scopeQuery = <T = unknown[], P = Record<string, string>>(queryRef: CBLQueryRef<T, P>): ScopedQuery<T, P> => ({
  addChangeListener: (handler: QueryChangeListener<T>) => addQueryChangeListener(queryRef, handler),
  execute: () => executeQuery(queryRef),
  explain: explainQuery.bind(null, queryRef),
  getParameters: () => getQueryParameters(queryRef),
  setParameters: setQueryParameters.bind(null, queryRef)
})

export type ScopedDatabase = ReturnType<typeof scopeDatabase>
