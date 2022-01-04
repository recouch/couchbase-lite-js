import { CBL } from './CBL'
import { Document } from './Document'
import { ExternalRef } from './ExternalRef'

type CBLDatabaseRef = ExternalRef<'CBLDatabaseRef'>

interface ConstructorParams {
  name: string
  ref: CBLDatabaseRef
}

export class Database {
  #ref: CBLDatabaseRef | undefined;
  readonly name: string;

  get ref(): CBLDatabaseRef | undefined {
    return this.#ref
  }

  private constructor({ name, ref }: ConstructorParams) {
    this.#ref = ref
    this.name = name
  }

  // Database lifecycle operations
  close() {
    if (!this.#ref) throw (new Error('Cannot close a closed database'))

    CBL.Database_Close(this.ref)
    this.#ref = undefined
  }

  delete() {
    if (!this.#ref) throw (new Error('Cannot delete a closed database'))

    CBL.Database_Delete(this.ref)
    this.#ref = undefined
  }

  addChangeListener(changeHandler: (docIDs: string[]) => void): (() => void) {
    if (!this.#ref) throw (new Error('Cannot listen to changes on a closed database'))

    return CBL.Database_AddChangeListener(this.ref, changeHandler)
  }

  // Document operations
  getDocument<T extends Record<string, unknown> = Record<string, unknown>>(id: string): Document<T> | null {
    if (!this.#ref) throw (new Error('Cannot get a document from a closed database'))

    const ref = CBL.Database_GetDocument(this.ref, id)

    return ref ? new Document({ database: this, id, ref, saved: true }) : null
  }

  saveDocument(document: Document): boolean {
    if (!this.#ref) throw (new Error('Cannot save a document to a closed database'))

    return CBL.Database_SaveDocument(this.ref, document.ref, JSON.stringify(document.value))
  }

  static open(name: string): Database {
    const ref = CBL.Database_Open(name)

    return new Database({ name, ref })
  }
}
