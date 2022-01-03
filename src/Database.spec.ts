import fs from 'fs'
import { join } from 'path'
import { Database } from './Database'
import { Document } from './Document'
import { removeListener } from './Listener'

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
      expect(db.delete()).toBe(true)
    })

    it('opens an existing database', () => {
      const dbName = 'existing_db'

      Database.open(dbName).close()

      const db = Database.open(dbName)

      expect(db).toBeTruthy()
      expect(db.delete()).toBe(true)
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
  })
})
