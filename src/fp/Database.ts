import { CBL } from '../CBL'

type DatabaseChangeListener = (docIDs: string[]) => void
type RemoveDatabaseChangeListener = () => void

export interface CBLDatabaseRef extends Symbol {
  type: 'Database'
}

export function addDatabaseChangeListener(db: CBLDatabaseRef, handler: DatabaseChangeListener): RemoveDatabaseChangeListener {
  return CBL.Database_AddChangeListener(db, handler)
}

export function beginTransaction(db: CBLDatabaseRef): boolean {
  return CBL.Database_BeginTransaction(db)
}

export function closeDatabase(db: CBLDatabaseRef): boolean {
  return CBL.Database_Close(db)
}

export function databaseName(db: CBLDatabaseRef): string {
  return CBL.Database_Name(db)
}

export function databasePath(db: CBLDatabaseRef): string {
  return CBL.Database_Path(db)
}

export function deleteDatabase(db: CBLDatabaseRef): boolean {
  return CBL.Database_Delete(db)
}

export function deleteDatabaseByName(name: string, directory: string): boolean {
  return CBL.DeleteDatabase(name, directory)
}

export function endTransaction(db: CBLDatabaseRef, commit: boolean): boolean {
  return CBL.Database_EndTransaction(db, commit)
}

export function openDatabase(name: string, directory?: string): CBLDatabaseRef {
  return CBL.Database_Open(name, directory)
}
