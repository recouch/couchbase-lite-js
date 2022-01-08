import { CBL } from './CBL'
import { Database } from './Database'
import { ExternalRef } from './ExternalRef'

type CBLQueryRef = ExternalRef<'CBLQueryRef'>

interface QueryConstructorParams {
  database: Database
  ref: CBLQueryRef
}

export enum QueryLanguage {
  JSON,
  N1QL
}

export class Query {
  database: Database
  #ref: CBLQueryRef | undefined;

  get ref(): CBLQueryRef | undefined {
    return this.#ref
  }

  private constructor({ database, ref }: QueryConstructorParams) {
    this.database = database
    this.#ref = ref
  }

  execute() {
    if (!this.#ref) throw (new Error('Cannot execute a released query'))

    const results: string = CBL.Query_Execute(this.ref)

    return JSON.parse(results)
  }

  explain() {
    if (!this.#ref) throw (new Error('Cannot explain a released query'))

    return CBL.Query_Explain(this.ref)
  }

  release() {
    if (!this.ref) throw new Error('Cannot release a released query')

    CBL.Document_Release(this.ref)
    this.#ref = undefined
  }

  // Change listeners
  addChangeListener(changeHandler: (changes: unknown[]) => void) {
    if (!this.#ref) throw (new Error('Cannot listen to changes on a released query'))

    const cb = (data: string) => changeHandler(JSON.parse(data))

    return CBL.Query_AddChangeListener(this.ref, cb)
  }

  static create(database: Database, query: string | unknown[]): Query {
    const ref: CBLQueryRef =
      typeof query === 'string'
        ? CBL.Database_CreateQuery(database.ref, QueryLanguage.N1QL, query)
        : CBL.Database_CreateQuery(database.ref, QueryLanguage.JSON, JSON.stringify(query))

    return new Query({ database, ref })
  }
}
