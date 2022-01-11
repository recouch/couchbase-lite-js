import { CBL } from '../CBL'
import { CBLDatabaseRef } from './Database'

export declare interface CBLQueryRef extends Symbol {
  type: 'Query'
}

enum QueryLanguage {
  JSON,
  N1QL
}

type QueryChangeListener<T> = (results: T[]) => void
type RemoveQueryChangeListener = () => void

export function addQueryChangeListener<T = unknown>(query: CBLQueryRef, handler: QueryChangeListener<T>): RemoveQueryChangeListener {
  const cb = (data: string) => handler(JSON.parse(data))

  return CBL.Query_AddChangeListener(query, cb)
}

export function createQuery(db: CBLDatabaseRef, query: string | unknown[]): CBLQueryRef {
  return typeof query === 'string'
    ? CBL.Database_CreateQuery(db, QueryLanguage.N1QL, query)
    : CBL.Database_CreateQuery(db, QueryLanguage.JSON, JSON.stringify(query))
}

export function executeQuery(query: CBLQueryRef): CBLQueryRef {
  return JSON.parse(CBL.Query_Execute(query))
}

export function explainQuery(query: CBLQueryRef): CBLQueryRef {
  return CBL.Query_Explain(query)
}
