import { CBL } from '../CBL'

export function addDocumentChangeListener(db: CBL.DatabaseRef, docID: string, handler: CBL.DocumentChangeListener): CBL.RemoveDocumentChangeListener {
  return CBL.Database_AddDocumentChangeListener(db, docID, handler)
}

export function createDocument<T = unknown>(id?: string): CBL.MutableDocumentRef<T> {
  return id ? CBL.Document_CreateWithID(id) : CBL.Document_Create()
}

export function deleteDocument(db: CBL.DatabaseRef, doc: CBL.DocumentRef | CBL.MutableDocumentRef): boolean {
  return CBL.Database_DeleteDocument(db, doc)
}

export function getDocument<T = unknown>(db: CBL.DatabaseRef, id: string): CBL.DocumentRef<T> | null {
  return CBL.Database_GetDocument(db, id)
}

export function getDocumentID(doc: CBL.DocumentRef | CBL.MutableDocumentRef): string {
  return CBL.Document_ID(doc)
}

export function getDocumentProperties<T = unknown>(doc: CBL.DocumentRef<T> | CBL.MutableDocumentRef<T>): T {
  return JSON.parse(CBL.Document_CreateJSON(doc))
}

export function getMutableDocument<T = unknown>(db: CBL.DatabaseRef, id: string): CBL.MutableDocumentRef<T> | null {
  return CBL.Database_GetMutableDocument(db, id)
}

export function saveDocument(db: CBL.DatabaseRef, doc: CBL.MutableDocumentRef): boolean {
  return CBL.Database_SaveDocument(db, doc)
}

export function setDocumentProperties<T>(doc: CBL.MutableDocumentRef<T>, value: T): boolean {
  return CBL.Document_SetJSON(doc, JSON.stringify(value))
}
