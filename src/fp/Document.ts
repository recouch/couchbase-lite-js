import { CBL } from '../CBL'
import { DatabaseRef, DocumentChangeListener, DocumentRef, MutableDocumentRef, RemoveDocumentChangeListener } from '../types'

export function addDocumentChangeListener(db: DatabaseRef, docID: string, handler: DocumentChangeListener): RemoveDocumentChangeListener {
  return CBL.Database_AddDocumentChangeListener(db, docID, handler)
}

export function createDocument<T = unknown>(id?: string): MutableDocumentRef<T> {
  return id ? CBL.Document_CreateWithID(id) : CBL.Document_Create()
}

export function deleteDocument(db: DatabaseRef, doc: DocumentRef | MutableDocumentRef): boolean {
  return CBL.Database_DeleteDocument(db, doc)
}

export function getDocument<T = unknown>(db: DatabaseRef, id: string): DocumentRef<T> | null {
  return CBL.Database_GetDocument(db, id)
}

export function getDocumentID(doc: DocumentRef | MutableDocumentRef): string {
  return CBL.Document_ID(doc)
}

export function getDocumentJSON<T = unknown>(doc: DocumentRef<T> | MutableDocumentRef<T>): string {
  return CBL.Document_CreateJSON(doc)
}

export function getDocumentProperties<T = unknown>(doc: DocumentRef<T> | MutableDocumentRef<T>): T {
  return CBL.Document_Properties(doc)
}

export function getMutableDocument<T = unknown>(db: DatabaseRef, id: string): MutableDocumentRef<T> | null {
  return CBL.Database_GetMutableDocument(db, id)
}

export function saveDocument(db: DatabaseRef, doc: MutableDocumentRef): boolean {
  return CBL.Database_SaveDocument(db, doc)
}

export function setDocumentJSON<T>(doc: MutableDocumentRef<T>, value: string): boolean {
  return CBL.Document_SetJSON(doc, value)
}

export function setDocumentProperties<T>(doc: MutableDocumentRef<T>, value: T): boolean {
  return CBL.Document_SetProperties(doc, value)
}
