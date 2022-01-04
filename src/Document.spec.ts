import { Database } from './Database'
import { Document } from './Document'

describe('Database', () => {
  describe('create', () => {
    it('creates a new document with an auto-generated ID', () => {
      const db = Database.open('new_doc_db')
      const doc = Document.create(db)

      expect(doc.value).toEqual({})
      expect(doc.id).toMatch(/^~/)

      doc.release()
      db.delete()
    })

    it('creates a new document with a custom ID', () => {
      const db = Database.open('new_doc_db')
      const doc = Document.create(db, 'custom-id')

      expect(doc.value).toEqual({})
      expect(doc.id).toBe('custom-id')

      doc.release()
      db.delete()
    })
  })

  describe('get id', () => {
    it('does not allow getting the id of a released document', () => {
      const db = Database.open('get_value_db')
      const doc = Document.create(db)

      doc.release()

      expect(() => doc.id).toThrowError('Cannot get ID of a released document')
      db.delete()
    })
  })

  describe('get value', () => {
    it('does not allow getting data from a released document', () => {
      const db = Database.open('get_value_db')
      const doc = Document.create(db)

      doc.release()

      expect(() => doc.value).toThrowError('Cannot get value of a released document')
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

    it('does not allow setting data on a released document', () => {
      const db = Database.open('set_value_db')
      const doc = Document.create(db)

      doc.release()

      expect(() => { doc.value = { testing: true } }).toThrowError('Cannot set value of a released document')
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
      expect(doc2?.value).toEqual({ testing: true })

      doc2?.release()
      db.delete()
    })

    it('does not allow saving a released document', () => {
      const db = Database.open('save_db')
      const doc = Document.create(db)

      doc.release()

      expect(() => doc.save()).toThrowError('Cannot save a released document')

      db.delete()
    })
  })

  describe('delete', () => {
    it('deletes the document', () => {
      const db = Database.open('delete_db')
      const doc = Document.create(db, 'deleted_doc')

      doc.value = { testing: true }
      doc.save()
      doc.delete()

      const doc2 = db.getDocument('deleted_doc')
      expect(doc2).toBeNull()

      db.delete()
    })

    it('throws an error when trying to delete an unsaved document', () => {
      const db = Database.open('delete_db')
      const doc = Document.create(db)

      doc.value = { testing: true }

      expect(() => doc.delete()).toThrowError('Cannot delete an unsaved document')

      doc.release()
      db.delete()
    })

    it('does not allow deleting a released document', () => {
      const db = Database.open('delete_db')
      const doc = Document.create(db)

      doc.release()

      expect(() => doc.delete()).toThrowError('Cannot delete a released document')

      db.delete()
    })
  })

  describe('release', () => {
    it('does not allow releasing a document that is already released', () => {
      const db = Database.open('release_doc')
      const doc = Document.create(db)

      doc.release()

      expect(() => doc.release()).toThrowError('Cannot release a released document')

      db.delete()
    })
  })
})
