import { CBL } from '../CBL'
import { CBLDatabaseRef } from './Database'

// eslint-disable-next-line @typescript-eslint/no-empty-interface, @typescript-eslint/no-unused-vars
export declare interface CBLQueryRef<T = unknown, P = Record<string, string>> extends Symbol {
  type: 'Query'
}

enum QueryLanguage {
  JSON,
  N1QL
}

type QueryChangeListener<T> = (results: T[]) => void
type RemoveQueryChangeListener = () => void

export function addQueryChangeListener<T = unknown>(query: CBLQueryRef<T>, handler: QueryChangeListener<T>): RemoveQueryChangeListener {
  const cb = (data: string) => handler(JSON.parse(data) as T[])

  return CBL.Query_AddChangeListener(query, cb)
}

export function createQuery<T = unknown, P = Record<string, string>>(db: CBLDatabaseRef, query: string | unknown[]): CBLQueryRef<T, P> {
  return typeof query === 'string'
    ? CBL.Database_CreateQuery(db, QueryLanguage.N1QL, query)
    : CBL.Database_CreateQuery(db, QueryLanguage.JSON, JSON.stringify(query))
}

export function executeQuery<T = unknown>(query: CBLQueryRef<T>): T[] {
  return JSON.parse(CBL.Query_Execute(query))
}

export function explainQuery(query: CBLQueryRef): string {
  return CBL.Query_Explain(query)
}

export function getQueryParameters<T = unknown, P = Record<string, string>>(query: CBLQueryRef<T, P>): P {
  return CBL.Query_Parameters(query)
}

export function setQueryParameters<T = unknown, P = Record<string, string>>(query: CBLQueryRef<T, P>, parameters: Partial<P>): boolean {
  return CBL.Query_SetParameters(query, parameters)
}
