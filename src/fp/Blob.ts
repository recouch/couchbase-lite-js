import { CBL } from '../CBL'
import { BlobRef, BlobMetadata, BlobWriteStreamRef, BlobReadStreamRef, DatabaseRef, DocumentRef, MutableDocumentRef } from '../types'

export function blobContent(blob: BlobRef): Buffer {
  return CBL.Blob_Content(blob)
}

export function blobContentType(blob: BlobRef): string {
  return CBL.Blob_ContentType(blob)
}

export function blobCreateJson(blob: BlobRef): string {
  return CBL.Blob_CreateJSON(blob)
}

export function blobProperties(blob: BlobRef): BlobMetadata {
  return JSON.parse(CBL.Blob_CreateJSON(blob))
}

export function createBlobWithData(contentType: string, buffer: Buffer): BlobRef {
  return CBL.Blob_CreateWithData(contentType, buffer)
}

export function createBlobWithStream(contentType: string, stream: BlobWriteStreamRef): BlobRef {
  return CBL.Blob_CreateWithStream(contentType, stream)
}

export function blobDigest(blob: BlobRef): string {
  return CBL.Blob_Digest(blob)
}

export function blobEquals(blob: BlobRef, anotherBlob: BlobRef): boolean {
  return CBL.Blob_Equals(blob, anotherBlob)
}

export function blobLength(blob: BlobRef): number {
  return CBL.Blob_Length(blob)
}

export function openBlobContentStream(blob: BlobRef): BlobReadStreamRef {
  return CBL.Blob_OpenContentStream(blob)
}

export function closeBlobReader(stream: BlobReadStreamRef): void {
  return CBL.BlobReader_Close(stream)
}

export function readBlobReader(stream: BlobReadStreamRef, maxLength: number): Buffer {
  return CBL.BlobReader_Read(stream, maxLength)
}

export function closeBlobWriter(stream: BlobWriteStreamRef): void {
  return CBL.BlobWriter_Close(stream)
}

export function createBlobWriter(database: DatabaseRef): BlobWriteStreamRef {
  return CBL.BlobWriter_Create(database)
}

export function writeBlobWriter(stream: BlobWriteStreamRef, buffer: Buffer): boolean {
  return CBL.BlobWriter_Write(stream, buffer)
}

export function databaseGetBlob(database: DatabaseRef, properties: BlobMetadata): BlobRef {
  return CBL.Database_GetBlob(database, JSON.stringify(properties))
}

export function databaseSaveBlob(database: DatabaseRef, blob: BlobRef): boolean {
  return CBL.Database_SaveBlob(database, blob)
}

export function documentGetBlob(doc: DocumentRef | MutableDocumentRef, property: string): BlobRef {
  return CBL.Document_GetBlob(doc, property)
}

export function documentIsBlob(doc: DocumentRef | MutableDocumentRef, property: string): boolean {
  return CBL.Document_IsBlob(doc, property)
}

export function documentSetBlob(doc: MutableDocumentRef, property: string, blob: BlobRef): void {
  return CBL.Document_SetBlob(doc, property, blob)
}
