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
    const results: string = CBL.Query_Execute(this.ref)

    return JSON.parse(results)
  }

  explain() {
    return CBL.Query_Explain(this.ref)
  }

  release() {
    if (!this.ref) throw new Error('Cannot release a released query')

    CBL.Document_Release(this.ref)
    this.#ref = undefined
  }

  static create(database: Database, query: string | unknown[]): Query {
    const ref: CBLQueryRef =
      typeof query === 'string'
        ? CBL.Database_CreateQuery(database.ref, QueryLanguage.N1QL, query)
        : CBL.Database_CreateQuery(database.ref, QueryLanguage.JSON, JSON.stringify(query))

    return new Query({ database, ref })
  }
}
