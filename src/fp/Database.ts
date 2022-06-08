import { CBL } from '../CBL'
import { DatabaseChangeListener, DatabaseRef, RemoveDatabaseChangeListener } from '../types'

export function abortTransaction(db: DatabaseRef): boolean {
  return CBL.Database_EndTransaction(db, false)
}

export function addDatabaseChangeListener(db: DatabaseRef, handler: DatabaseChangeListener): RemoveDatabaseChangeListener {
  return CBL.Database_AddChangeListener(db, handler)
}

export function beginTransaction(db: DatabaseRef): boolean {
  return CBL.Database_BeginTransaction(db)
}

export function closeDatabase(db: DatabaseRef): boolean {
  return CBL.Database_Close(db)
}

export function commitTransaction(db: DatabaseRef): boolean {
  return CBL.Database_EndTransaction(db, true)
}

export function databaseName(db: DatabaseRef): string {
  return CBL.Database_Name(db)
}

export function databasePath(db: DatabaseRef): string {
  return CBL.Database_Path(db)
}

export function deleteDatabase(db: DatabaseRef): boolean {
  return CBL.Database_Delete(db)
}

export function deleteDatabaseByName(name: string, directory: string): boolean {
  return CBL.DeleteDatabase(name, directory)
}

export function endTransaction(db: DatabaseRef, commit: boolean): boolean {
  return CBL.Database_EndTransaction(db, commit)
}

export function openDatabase(name: string, directory?: string): DatabaseRef {
  return CBL.Database_Open(name, directory)
}
