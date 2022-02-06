import { CBL } from '../CBL'

export function abortTransaction(db: CBL.DatabaseRef): boolean {
  return CBL.Database_EndTransaction(db, false)
}

export function addDatabaseChangeListener(db: CBL.DatabaseRef, handler: CBL.DatabaseChangeListener): CBL.RemoveDatabaseChangeListener {
  return CBL.Database_AddChangeListener(db, handler)
}

export function beginTransaction(db: CBL.DatabaseRef): boolean {
  return CBL.Database_BeginTransaction(db)
}

export function closeDatabase(db: CBL.DatabaseRef): boolean {
  return CBL.Database_Close(db)
}

export function commitTransaction(db: CBL.DatabaseRef): boolean {
  return CBL.Database_EndTransaction(db, true)
}

export function databaseName(db: CBL.DatabaseRef): string {
  return CBL.Database_Name(db)
}

export function databasePath(db: CBL.DatabaseRef): string {
  return CBL.Database_Path(db)
}

export function deleteDatabase(db: CBL.DatabaseRef): boolean {
  return CBL.Database_Delete(db)
}

export function deleteDatabaseByName(name: string, directory: string): boolean {
  return CBL.DeleteDatabase(name, directory)
}

export function endTransaction(db: CBL.DatabaseRef, commit: boolean): boolean {
  return CBL.Database_EndTransaction(db, commit)
}

export function openDatabase(name: string, directory?: string): CBL.DatabaseRef {
  return CBL.Database_Open(name, directory)
}
