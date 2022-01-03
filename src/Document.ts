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
  readonly ref: CBLDocumentRef
  #id: string
  #value: T

  get id(): string {
    this.#id ??= this.#getID()

    return this.#id
  }

  get value(): T {
    this.#value ??= this.#getValue()

    return this.#value
  }

  set value(value: T) {
    this.#setValue(value)
    this.#value = value
  }

  constructor({ database, id, ref }: ConstructorParams) {
    this.database = database
    this.ref = ref
    id && (this.#id = id)
  }

  #getID(): string {
    return CBL.Document_ID(this.ref)
  }

  #getValue(): T {
    return JSON.parse(CBL.Document_CreateJSON(this.ref))
  }

  #setValue(value: T) {
    return CBL.Document_SetJSON(this.ref, JSON.stringify(value))
  }

  delete(): boolean {
    return CBL.Database_DeleteDocument(this.database.ref, this.ref)
  }

  /**
   * Releases the document from memory
   * The document cannot be mutated or saved after calling this
   */
  release(): boolean {
    return CBL.Document_Release(this.ref)
  }

  save(): boolean {
    return this.database.saveDocument(this)
  }

  static create(database: Database, id?: string): Document {
    const ref = id ? CBL.Document_CreateWithID(id) : CBL.Document_Create()

    return new Document({ database, id, ref })
  }
}
