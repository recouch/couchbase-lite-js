import { CBL } from '../CBL'

export function blobContent(blob: CBL.BlobRef): Buffer {
  return CBL.Blob_Content(blob)
}

export function blobContentType(blob: CBL.BlobRef): string {
  return CBL.Blob_ContentType(blob)
}

export function blobCreateJson(blob: CBL.BlobRef): string {
  return CBL.Blob_CreateJSON(blob)
}

export function blobProperties(blob: CBL.BlobRef): CBL.BlobMetadata {
  return JSON.parse(CBL.Blob_CreateJSON(blob))
}

export function createBlobWithData(contentType: string, buffer: Buffer): CBL.BlobRef {
  return CBL.Blob_CreateWithData(contentType, buffer)
}

export function createBlobWithStream(contentType: string, stream: CBL.BlobWriteStreamRef): CBL.BlobRef {
  return CBL.Blob_CreateWithStream(contentType, stream)
}

export function blobDigest(blob: CBL.BlobRef): string {
  return CBL.Blob_Digest(blob)
}

export function blobEquals(blob: CBL.BlobRef, anotherBlob: CBL.BlobRef): boolean {
  return CBL.Blob_Equals(blob, anotherBlob)
}

export function blobLength(blob: CBL.BlobRef): number {
  return CBL.Blob_Length(blob)
}

export function openBlobContentStream(blob: CBL.BlobRef): CBL.BlobReadStreamRef {
  return CBL.Blob_OpenContentStream(blob)
}

export function closeBlobReader(stream: CBL.BlobReadStreamRef): void {
  return CBL.BlobReader_Close(stream)
}

export function readBlobReader(stream: CBL.BlobReadStreamRef, maxLength: number): Buffer {
  return CBL.BlobReader_Read(stream, maxLength)
}

export function closeBlobWriter(stream: CBL.BlobWriteStreamRef): void {
  return CBL.BlobWriter_Close(stream)
}

export function createBlobWriter(database: CBL.DatabaseRef): CBL.BlobWriteStreamRef {
  return CBL.BlobWriter_Create(database)
}

export function writeBlobWriter(stream: CBL.BlobWriteStreamRef, buffer: Buffer): boolean {
  return CBL.BlobWriter_Write(stream, buffer)
}

export function databaseGetBlob(database: CBL.DatabaseRef, properties: CBL.BlobMetadata): CBL.BlobRef {
  return CBL.Database_GetBlob(database, JSON.stringify(properties))
}

export function databaseSaveBlob(database: CBL.DatabaseRef, blob: CBL.BlobRef): boolean {
  return CBL.Database_SaveBlob(database, blob)
}

export function documentGetBlob(doc: CBL.DocumentRef | CBL.MutableDocumentRef, property: string): CBL.BlobRef {
  return CBL.Document_GetBlob(doc, property)
}

export function documentIsBlob(doc: CBL.DocumentRef | CBL.MutableDocumentRef, property: string): boolean {
  return CBL.Document_IsBlob(doc, property)
}

export function documentSetBlob(doc: CBL.MutableDocumentRef, property: string, blob: CBL.BlobRef): void {
  return CBL.Document_SetBlob(doc, property, blob)
}
