import fs from 'fs'
import { join } from 'path'
import { nanoid } from 'nanoid'
import { addDatabaseChangeListener, beginTransaction, closeDatabase, databaseName, databasePath, deleteDatabase, deleteDatabaseByName, endTransaction, openDatabase } from './Database'
import { createDocument, getDocument, getDocumentID, getDocumentProperties, saveDocument, setDocumentProperties } from './Document'
import { createTestDatabase, testDirectory, timeout } from './test-util'

describe('database functions', () => {
  describe('addDatabaseChangeListener', () => {
    it('calls callback on change', async () => {
      const { cleanup, db } = createTestDatabase()
      const doc = createDocument()
      const fn = jest.fn()
      const stop = addDatabaseChangeListener(db, fn)

      expect(typeof stop).toBe('function')

      await timeout()
      expect(fn).not.toHaveBeenCalled()

      saveDocument(db, doc)

      await timeout()
      expect(fn).toHaveBeenCalledWith([getDocumentID(doc)])

      expect(stop()).toBe(true)

      cleanup()
    })
  })

  describe('closeDatabase', () => {
    it('closes the database', async () => {
      const { db } = createTestDatabase()
      const doc = createDocument()

      expect(closeDatabase(db)).toBe(true)
      expect(() => saveDocument(db, doc)).toThrowError('Database is closed')
    })

    it('allows multiple calls on the same database', () => {
      const { cleanup, db } = createTestDatabase()

      expect(closeDatabase(db)).toBe(true)
      expect(closeDatabase(db)).toBe(true)

      cleanup()
    })
  })

  describe('deleteDatabase', () => {
    it('deletes the database', () => {
      const { cleanup, db, dbPath } = createTestDatabase()

      expect(deleteDatabase(db)).toBe(true)
      expect(fs.existsSync(dbPath)).toBe(false)

      cleanup()
    })

    it('allows multiple calls on the same database', () => {
      const { cleanup, db } = createTestDatabase()

      expect(deleteDatabase(db)).toBe(true)
      expect(deleteDatabase(db)).toBe(true)

      cleanup()
    })
  })

  describe('deleteDatabaseByName', () => {
    it('deletes the database', () => {
      const { cleanup, dbName, dbPath, db } = createTestDatabase()

      closeDatabase(db)
      expect(fs.existsSync(dbPath)).toBe(true)
      expect(deleteDatabaseByName(dbName, testDirectory)).toBe(true)
      expect(fs.existsSync(dbPath)).toBe(false)

      cleanup()
    })

    it('does not throw when database does not exist', () => {
      expect(deleteDatabaseByName(`non-existent-db-${nanoid()}`, testDirectory)).toBe(true)
    })

    it('throws an error when the database is still open', () => {
      const { cleanup, dbName, dbPath } = createTestDatabase()

      expect(() => deleteDatabaseByName(dbName, testDirectory)).toThrowError("Can't delete db file while other connections are open. The open connections are tagged appOpened.")
      expect(fs.existsSync(dbPath)).toBe(true)

      cleanup()
    })
  })

  describe('databaseName', () => {
    it('gets the name of the reffed database', () => {
      const db = openDatabase('some_db')

      expect(databaseName(db)).toBe('some_db')

      deleteDatabase(db)
    })
  })

  describe('databasePath', () => {
    it('gets the path of the reffed database', () => {
      const db = openDatabase('some_db')

      expect(databasePath(db)).toBe(join(__dirname, '../../some_db.cblite2/'))

      deleteDatabase(db)
    })
  })

  describe('openDatabase', () => {
    it('creates a new database on disk', () => {
      const db = openDatabase('new_db')

      expect(db).toBeTruthy()
      const dbPath = join(__dirname, '../../new_db.cblite2')
      expect(fs.statSync(dbPath).isDirectory()).toBe(true)
      expect(databaseName(db)).toBe('new_db')

      deleteDatabase(db)
    })

    it('opens an existing database', () => {
      const dbName = 'existing_db'

      closeDatabase(openDatabase(dbName, testDirectory))
      const db = openDatabase(dbName, testDirectory)
      expect(db).toBeTruthy()

      deleteDatabase(db)
    })
  })

  describe('beginTransaction/endTransaction', () => {
    it('commits all changes at once when committing a transaction', () => {
      const { cleanup, db, dbName } = createTestDatabase()
      const db2 = openDatabase(dbName, testDirectory)

      expect(getDocument(db, 'doc1')).toBeNull()
      expect(getDocument(db, 'doc2')).toBeNull()

      expect(beginTransaction(db)).toBe(true)

      const doc1 = createDocument('doc1')
      const doc2 = createDocument('doc2')
      setDocumentProperties(doc1, { name: 'one' })
      setDocumentProperties(doc2, { name: 'two' })
      saveDocument(db, doc1)
      saveDocument(db, doc2)

      expect(getDocumentProperties(getDocument(db, 'doc1')!)).toEqual({ name: 'one' })
      expect(getDocumentProperties(getDocument(db, 'doc2')!)).toEqual({ name: 'two' })
      expect(getDocument(db2, 'doc1')).toBeNull()
      expect(getDocument(db2, 'doc2')).toBeNull()

      expect(endTransaction(db, true)).toBe(true)

      expect(getDocumentProperties(getDocument(db, 'doc1')!)).toEqual({ name: 'one' })
      expect(getDocumentProperties(getDocument(db, 'doc2')!)).toEqual({ name: 'two' })
      expect(getDocumentProperties(getDocument(db2, 'doc1')!)).toEqual({ name: 'one' })
      expect(getDocumentProperties(getDocument(db2, 'doc2')!)).toEqual({ name: 'two' })

      closeDatabase(db2)
      cleanup()
    })

    it('reverts all changes when ending a transaction without committing', () => {
      const { cleanup, db, dbName } = createTestDatabase()
      const db2 = openDatabase(dbName, testDirectory)

      expect(getDocument(db, 'doc1')).toBeNull()
      expect(getDocument(db, 'doc2')).toBeNull()

      expect(beginTransaction(db)).toBe(true)

      const doc1 = createDocument('doc1')
      const doc2 = createDocument('doc2')
      setDocumentProperties(doc1, { name: 'one' })
      setDocumentProperties(doc2, { name: 'two' })
      saveDocument(db, doc1)
      saveDocument(db, doc2)

      expect(getDocumentProperties(getDocument(db, 'doc1')!)).toEqual({ name: 'one' })
      expect(getDocumentProperties(getDocument(db, 'doc2')!)).toEqual({ name: 'two' })
      expect(getDocument(db2, 'doc1')).toBeNull()
      expect(getDocument(db2, 'doc2')).toBeNull()

      expect(endTransaction(db, false)).toBe(true)

      expect(getDocument(db, 'doc1')!).toBeNull()
      expect(getDocument(db, 'doc2')!).toBeNull()
      expect(getDocument(db2, 'doc1')!).toBeNull()
      expect(getDocument(db2, 'doc2')!).toBeNull()

      closeDatabase(db2)
      cleanup()
    })
  })
})
