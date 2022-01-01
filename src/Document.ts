import { CBL } from './CBL'
import type { Database } from './Database'

/**
 * Couchbase Lite document reference
 * Only usable in C
 * The private #external field ensures that other types cannot be infered as CBLDocumentRef
 * The private constructor ensures that CBLDocumentRef cannot be instantiated
 */
class CBLDocumentRef {
  #external = true
  private constructor() { /* noop */ }
}

interface ConstructorParams {
  database: Database
  id?: string
  ref: CBLDocumentRef
}

export class Document<T extends Record<string, unknown> = Record<string, unknown>> {
  database: Database
  readonly id?: string
  readonly ref: CBLDocumentRef
  value: T

  constructor({ database, id, ref }: ConstructorParams) {
    this.database = database
    this.ref = ref
    this.id = id
  }

  delete(): boolean {
    return CBL.Database_DeleteDocument(this.database.ref, this.ref)
  }

  save(): boolean {
    return this.database.saveDocument(this)
  }

  static create(database: Database, id?: string): Document {
    const ref = id ? CBL.Document_CreateWithID(id) : CBL.Document_Create()

    return new Document({ database, id, ref })
  }
}
