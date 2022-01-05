import { CBL } from './CBL'
import type { Database } from './Database'
import { ExternalRef } from './ExternalRef'

type CBLDocumentRef = ExternalRef<'CBLDocumentRef'>

interface DocumentConstructorParams {
  database: Database
  id?: string
  ref: CBLDocumentRef
  saved: boolean
}

interface MutableDocumentConstructorParams extends DocumentConstructorParams {
  saved: boolean
}

export class Document<T extends Record<string, unknown> = Record<string, unknown>> {
  database: Database
  #id: string
  #ref: CBLDocumentRef | undefined;
  #value: T | null = null

  get ref(): CBLDocumentRef | undefined {
    return this.#ref
  }

  get id(): string {
    this.#id ??= this.#getID()

    return this.#id
  }

  get value(): T {
    this.#value ??= this.#getValue()

    return this.#value
  }

  constructor({ database, id, ref }: DocumentConstructorParams) {
    this.database = database
    id && (this.#id = id)
    this.#ref = ref
  }

  #getID(): string {
    if (!this.ref) throw new Error('Cannot get ID of a released document')

    return CBL.Document_ID(this.ref)
  }

  #getValue(): T {
    if (!this.ref) throw new Error('Cannot get value of a released document')

    return JSON.parse(CBL.Document_CreateJSON(this.ref))
  }

  delete() {
    if (!this.ref) throw new Error('Cannot delete a released document')

    const res = CBL.Database_DeleteDocument(this.database.ref, this.ref)
    this.#ref = undefined

    return res
  }

  /**
   * Releases the document from memory
   * The document cannot be mutated or saved after calling this
   */
  release() {
    if (!this.ref) throw new Error('Cannot release a released document')

    CBL.Document_Release(this.ref)
    this.#ref = undefined
  }
}

export class MutableDocument<T extends Record<string, unknown> = Record<string, unknown>> extends Document<T> {
  #saved: boolean
  #value: T | null = null

  get saved(): boolean {
    return this.#saved
  }

  get value(): T {
    this.#value ??= this.#getValue()

    return this.#value
  }

  set value(value: T) {
    this.#setValue(value)
    this.#value = value
  }

  constructor(params: MutableDocumentConstructorParams) {
    super(params)
    this.#saved = params.saved
  }

  #getValue(): T {
    if (!this.ref) throw new Error('Cannot get value of a released document')

    return JSON.parse(CBL.Document_CreateJSON(this.ref))
  }

  #setValue(value: T) {
    if (!this.ref) throw new Error('Cannot set value of a released document')

    return CBL.Document_SetJSON(this.ref, JSON.stringify(value))
  }

  save() {
    if (!this.ref) throw new Error('Cannot save a released document')

    this.database.saveDocument(this)
    this.#saved = true
  }

  delete() {
    if (!this.#saved) throw new Error('Cannot delete an unsaved document')

    const res = super.delete()

    this.#saved = false

    return res
  }

  static create(database: Database, id?: string): MutableDocument {
    const ref = id ? CBL.Document_CreateWithID(id) : CBL.Document_Create()

    return new MutableDocument({ database, id, ref, saved: false })
  }
}
