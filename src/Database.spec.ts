import fs from 'fs'
import { join } from 'path'
import { Database } from './Database'
import { Document } from './Document'
import { removeListener } from './Listener'

const getDbPath = (db:Database) => join(__dirname, `../${db.name}.cblite2`)

describe('Database', () => {
  describe('open', () => {
    it('creates a new database on disk', () => {
      const dbName = 'new_db'
      const dbPath = join(__dirname, `../${dbName}.cblite2`)

      // Ensure db does not yet exist
      fs.existsSync(dbPath) && fs.rmSync(dbPath, { recursive: true })

      const db = Database.open(dbName)

      expect(db).toBeTruthy()
      expect(fs.statSync(dbPath).isDirectory()).toBe(true)
      db.delete()
    })

    it('opens an existing database', () => {
      const dbName = 'existing_db'

      Database.open(dbName).close()

      const db = Database.open(dbName)

      expect(db).toBeTruthy()
      db.delete()
    })
  })

  describe('close', () => {
    it('closes the database', () => {
      const db = Database.open('close_db')
      const doc = Document.create(db)

      expect(() => db.close()).not.toThrow()
      expect(() => doc.save()).toThrowError('Cannot save a document to a closed database')

      doc.release()

      fs.rmSync(getDbPath(db), { recursive: true })
    })

    it('does not allow operation on a closed database', () => {
      const db = Database.open('close_db')

      db.close()
      expect(() => db.close()).toThrowError('Cannot close a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })

  describe('delete', () => {
    it('deletes the database', () => {
      const db = Database.open('delete_db')

      expect(() => db.delete()).not.toThrow()
      expect(fs.existsSync(getDbPath(db))).toBe(false)
    })

    it('does not allow operation on a deleted database', () => {
      const db = Database.open('delete_db')

      db.delete()
      expect(() => db.delete()).toThrowError('Cannot delete a closed database')
    })
  })

  describe('addChangeListener', () => {
    it('calls callback on change', async () => {
      const db = Database.open('change_db')
      const doc = Document.create(db)
      const fn = jest.fn()
      const token = db.addChangeListener(fn)

      expect(fn).not.toHaveBeenCalled()

      await new Promise(resolve => setTimeout(resolve, 10))
      doc.save()

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).toHaveBeenCalledWith([doc.id])

      removeListener(token)
      doc.release()
      db.delete()
    })

    it('does not allow operation on a closed database', () => {
      const db = Database.open('change_db')

      db.close()
      expect(() => db.addChangeListener(jest.fn())).toThrowError('Cannot listen to changes on a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })

  describe('getDocument', () => {
    it('calls callback on change', () => {
      const db = Database.open('get_document_db')
      const newDoc = Document.create(db, 'boy')

      newDoc.value = { name: 'Milo' }
      newDoc.save()
      newDoc.release()

      const retrievedDoc = db.getDocument<{ name: string }>('boy')
      expect(retrievedDoc?.value.name).toBe('Milo')

      db.delete()
    })

    it('returns null when document is not found', () => {
      const db = Database.open('get_document_db')

      const retrievedDoc = db.getDocument<{ name: string }>('girl')
      expect(retrievedDoc).toBeNull()

      db.delete()
    })

    it('does not allow operation on a closed database', () => {
      const db = Database.open('get_document_db')

      db.close()
      expect(() => db.getDocument('son')).toThrowError('Cannot get a document from a closed database')

      fs.rmSync(getDbPath(db), { recursive: true })
    })
  })
})
