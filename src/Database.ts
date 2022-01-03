import { CBL } from './CBL'
import { Document } from './Document'

/**
 * Couchbase Lite database reference
 * Only usable in C
 * The private #external field ensures that other types cannot be infered as CBLDatabaseRef
 * The private constructor ensures that CBLDatabaseRef cannot be instantiated
 */
class CBLDatabaseRef {
  #external = true
  private constructor() { /* noop */ }
}

interface ConstructorParams {
  name: string
  ref: CBLDatabaseRef
}

export class Database {
  readonly ref: CBLDatabaseRef;
  readonly name: string;

  private constructor({ name, ref }: ConstructorParams) {
    this.ref = ref
    this.name = name
  }

  // Database lifecycle operations
  close(): boolean {
    try {
      return CBL.Database_Close(this.ref)
    } catch {
      return false
    }
  }

  delete(): boolean {
    try {
      return CBL.Database_Delete(this.ref)
    } catch {
      return false
    }
  }

  addChangeListener(changeHandler: (docIDs: string[]) => void): (() => void) {
    return CBL.Database_AddChangeListener(this.ref, changeHandler)
  }

  // Document operations
  getDocument(id: string): Document {
    const ref = CBL.Database_GetDocument(this.ref, id)

    return new Document({ database: this, id, ref })
  }

  saveDocument(document: Document): boolean {
    return CBL.Database_SaveDocument(this.ref, document.ref, JSON.stringify(document.value))
  }

  static open(name: string): Database {
    const ref = CBL.Database_Open(name)

    return new Database({ name, ref })
  }
}
