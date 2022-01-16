import fs from 'fs'
import { join } from 'path'
import { createTestDatabase, getDbPath, testDirectory } from './test-util'
import { Database } from './Database'
import { MutableDocument } from './Document'

describe('Database', () => {
  describe('open', () => {
    it('creates a new database on disk', () => {
      const db = Database.open('new_db')

      expect(db).toBeTruthy()
      const dbPath = join(__dirname, `../../${db.name}.cblite2`)
      expect(fs.statSync(dbPath).isDirectory()).toBe(true)

      db.delete()
    })

    it('opens an existing database', () => {
      const dbName = 'existing_db'

      Database.open(dbName, testDirectory).close()

      const db = Database.open(dbName, testDirectory)
      expect(db).toBeTruthy()

      db.delete()
    })
  })

  describe('close', () => {
    it('closes the database', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      expect(() => db.close()).not.toThrow()
      expect(() => doc.save()).toThrowError('Cannot save a document to a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })

    it('does not allow operation on a closed database', () => {
      const db = createTestDatabase()

      db.close()
      expect(() => db.close()).toThrowError('Cannot close a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })

  describe('delete', () => {
    it('deletes the database', () => {
      const db = createTestDatabase()

      expect(() => db.delete()).not.toThrow()
      expect(fs.existsSync(getDbPath(db))).toBe(false)
    })

    it('does not allow operation on a deleted database', () => {
      const db = createTestDatabase()

      db.delete()
      expect(() => db.delete()).toThrowError('Cannot delete a closed database')
    })
  })

  describe('addChangeListener', () => {
    it('calls callback on change', async () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)
      const fn = jest.fn()
      const stop = db.addChangeListener(fn)

      expect(typeof stop).toBe('function')

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).not.toHaveBeenCalled()

      doc.save()

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).toHaveBeenCalledWith([doc.id])

      expect(stop()).toBe(true)

      db.delete()
    })

    it('does not allow operation on a closed database', () => {
      const db = createTestDatabase()

      db.close()
      expect(() => db.addChangeListener(jest.fn())).toThrowError('Cannot listen to changes on a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })

  describe('getDocument', () => {
    it('gets a document by ID from the DB', () => {
      const db = createTestDatabase()
      const newDoc = MutableDocument.create(db, 'boy')

      newDoc.value = { name: 'Milo' }
      newDoc.save()

      const retrievedDoc = db.getDocument<{ name: string }>('boy')
      expect(retrievedDoc?.value.name).toBe('Milo')

      db.delete()
    })

    it('returns null when document is not found', () => {
      const db = createTestDatabase()

      const retrievedDoc = db.getDocument<{ name: string }>('girl')
      expect(retrievedDoc).toBeNull()

      db.delete()
    })

    it('does not allow operation on a closed database', () => {
      const db = createTestDatabase()

      db.close()
      expect(() => db.getDocument('son')).toThrowError('Cannot get a document from a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })

  describe('getMutableDocument', () => {
    it('calls callback on change', () => {
      const db = createTestDatabase()
      const newDoc = MutableDocument.create(db, 'boy')

      newDoc.value = { name: 'Milo' }
      newDoc.save()

      const retrievedDoc = db.getMutableDocument<{ name: string }>('boy')
      expect(retrievedDoc?.value.name).toBe('Milo')

      db.delete()
    })

    it('returns null when document is not found', () => {
      const db = createTestDatabase()

      const retrievedDoc = db.getMutableDocument('girl')
      expect(retrievedDoc).toBeNull()

      db.delete()
    })

    it('does not allow operation on a closed database', () => {
      const db = createTestDatabase()

      db.close()
      expect(() => db.getMutableDocument('son')).toThrowError('Cannot get a document from a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })

  describe('addDocumentChangeListener', () => {
    it('calls callback on change', async () => {
      const db = createTestDatabase({ testDoc: { children: 1 } })
      const doc = db.getMutableDocument('testDoc')!
      const fn = jest.fn()
      const stop = db.addDocumentChangeListener('testDoc', fn)

      expect(typeof stop).toBe('function')

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).not.toHaveBeenCalled()

      doc.value = { children: 2 }
      doc.save()

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).toHaveBeenCalledTimes(1)
      expect(fn).toHaveBeenCalledWith('testDoc')
      fn.mockClear()

      const doc2 = MutableDocument.create(db, 'testDoc2')
      doc2.value = { children: 0 }
      doc2.save()
      expect(fn).not.toHaveBeenCalled()

      expect(stop()).toBe(true)

      db.delete()
    })

    it('does not allow operation on a closed database', () => {
      const db = createTestDatabase()

      db.close()
      expect(() => db.addDocumentChangeListener('', jest.fn())).toThrowError('Cannot listen to document changes on a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })
})