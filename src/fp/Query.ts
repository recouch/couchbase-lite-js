import { CBL } from '../CBL'
import { DatabaseRef, QueryChangeListener, QueryRef, RemoveQueryChangeListener } from '../types'

export function addQueryChangeListener<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>, handler: QueryChangeListener<T>): RemoveQueryChangeListener {
  const cb = (data: string) => handler(JSON.parse(data) as T[])

  return CBL.Query_AddChangeListener(query, cb)
}

export function createQuery<T = unknown, P = Record<string, string>>(db: DatabaseRef, query: string | unknown[]): QueryRef<T, P> {
  // Remove unnecessary whitespace from N1QL queries, which cause runtime errors
  return typeof query === 'string'
    ? CBL.Database_CreateQuery(db, CBL.CBLN1QLLanguage, query)
    : CBL.Database_CreateQuery(db, CBL.CBLJSONLanguage, JSON.stringify(query))
}

export function executeQuery<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>): T[] {
  return JSON.parse(CBL.Query_Execute(query))
}

export function explainQuery<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>): string {
  return CBL.Query_Explain(query)
}

export function getQueryParameters<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>): Partial<P> {
  return JSON.parse(CBL.Query_Parameters(query))
}

export function setQueryParameters<T = unknown, P = Record<string, string>>(query: QueryRef<T, P>, parameters: Partial<P>): boolean {
  return CBL.Query_SetParameters(query, JSON.stringify(parameters))
}
