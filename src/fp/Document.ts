import { CBL } from '../CBL'
import { CBLDatabaseRef } from './Database'

// eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
export declare interface CBLDocumentRef<T = unknown> extends Symbol {
  type: 'Document',
  mutable: false
}

// eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
export declare interface CBLMutableDocumentRef<T = unknown> extends Symbol {
  type: 'MutableDocument',
  mutable: true
}

type DocumentChangeListener = (docID: string) => void
type RemoveDocumentChangeListener = () => void

export function addDocumentChangeListener(db: CBLDatabaseRef, docID: string, handler: DocumentChangeListener): RemoveDocumentChangeListener {
  return CBL.Database_AddDocumentChangeListener(db, docID, handler)
}

export function createDocument<T = unknown>(id?: string): CBLMutableDocumentRef<T> {
  return id ? CBL.Document_CreateWithID(id) : CBL.Document_Create()
}

export function deleteDocument<T>(db: CBLDatabaseRef, doc: CBLMutableDocumentRef<T>): boolean {
  return CBL.Database_DeleteDocument(db, doc)
}

export function getDocument<T = unknown>(db: CBLDatabaseRef, id: string): CBLDocumentRef<T> | null {
  return CBL.Database_GetDocument(db, id)
}

export function getDocumentID(doc: CBLDocumentRef | CBLMutableDocumentRef): string {
  return CBL.Document_ID(doc)
}

export function getDocumentProperties<T = unknown>(doc: CBLDocumentRef<T> | CBLMutableDocumentRef<T>): T {
  return JSON.parse(CBL.Document_CreateJSON(doc))
}

export function getMutableDocument<T = unknown>(db: CBLDatabaseRef, id: string): CBLMutableDocumentRef<T> | null {
  return CBL.Database_GetMutableDocument(db, id)
}

export function saveDocument<T>(db: CBLDatabaseRef, doc: CBLMutableDocumentRef<T>): boolean {
  return CBL.Database_SaveDocument(db, doc)
}

export function setDocumentProperties<T>(doc: CBLMutableDocumentRef<T>, value: T): boolean {
  return CBL.Document_SetJSON(doc, JSON.stringify(value))
}
