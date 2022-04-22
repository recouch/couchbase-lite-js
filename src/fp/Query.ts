import { CBL } from '../CBL'

export type QueryChangeListener<T> = (results: T[]) => void

export function addQueryChangeListener<T = unknown>(query: CBL.QueryRef<T>, handler: QueryChangeListener<T>): CBL.RemoveQueryChangeListener {
  const cb = (data: string) => handler(JSON.parse(data) as T[])

  return CBL.Query_AddChangeListener(query, cb)
}

export function createQuery<T = unknown, P = Record<string, string>>(db: CBL.DatabaseRef, query: string | unknown[]): CBL.QueryRef<T, P> {
  // Remove unnecessary whitespace from N1QL queries, which cause runtime errors
  return typeof query === 'string'
    ? CBL.Database_CreateQuery(db, CBL.CBLN1QLLanguage, query)
    : CBL.Database_CreateQuery(db, CBL.CBLJSONLanguage, JSON.stringify(query))
}

export function executeQuery<T = unknown>(query: CBL.QueryRef<T>): T[] {
  return JSON.parse(CBL.Query_Execute(query))
}

export function explainQuery(query: CBL.QueryRef): string {
  return CBL.Query_Explain(query)
}

export function getQueryParameters<T = unknown, P = Record<string, string>>(query: CBL.QueryRef<T, P>): P {
  return JSON.parse(CBL.Query_Parameters(query))
}

export function setQueryParameters<T = unknown, P = Record<string, string>>(query: CBL.QueryRef<T, P>, parameters: Partial<P>): boolean {
  return CBL.Query_SetParameters(query, JSON.stringify(parameters))
}
