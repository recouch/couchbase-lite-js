import { Database } from './Database'
import { Document } from './Document'

describe('Database', () => {
  describe('create', () => {
    it('creates a new document', () => {
      const db = Database.open('new_doc_db')
      const doc = Document.create(db)

      expect(doc.value).toEqual({})

      doc.release()
      db.delete()
    })
  })

  describe('set value', () => {
    it('sets the doc\'s value', () => {
      const db = Database.open('set_value_db')
      const doc = Document.create(db)

      doc.value = { testing: true }

      expect(doc.value).toEqual({ testing: true })

      doc.release()
      db.delete()
    })
  })

  describe('save', () => {
    it('persists the document', () => {
      const db = Database.open('save_db')
      const doc = Document.create(db, 'saved_doc')

      doc.value = { testing: true }
      doc.save()
      doc.release()

      const doc2 = db.getDocument('saved_doc')
      expect(doc2.value).toEqual({ testing: true })

      doc2.release()
      db.delete()
    })
  })
})
