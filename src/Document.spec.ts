import { createTestDatabase } from './test-util'
import { MutableDocument } from '.'

describe('Document', () => {
  describe('get value', () => {
    it('returns the value of the document', () => {
      const db = createTestDatabase({ 'test-1': { couch: 'potato' } })
      const doc = db.getDocument('test-1')!

      expect(doc.value).toEqual({ couch: 'potato' })

      // TODO: Find some way to ensure Document_CreateJSON is only called once
      expect(doc.value).toEqual({ couch: 'potato' })

      doc.release()
      db.delete()
    })

    it('does not allow getting data from a released document', () => {
      const db = createTestDatabase({ 'test-1': {} })
      const doc = db.getDocument('test-1')!

      doc.release()

      expect(() => doc.value).toThrowError('Cannot get value of a released document')
      db.delete()
    })
  })

  describe('set value', () => {
    it('does not allow setting data on a non-mutable document', () => {
      const db = createTestDatabase({ 'test-1': {} })
      const doc = db.getDocument('test-1')!

      // @ts-ignore
      expect(() => { doc.value = { testing: true } }).toThrow()

      doc.release()
      db.delete()
    })
  })

  describe('save', () => {
    it('does not allow saving a non-mutable document', () => {
      const db = createTestDatabase({ 'test-1': {} })
      const doc = db.getDocument('test-1')!

      doc.release()

      // @ts-ignore
      expect(() => doc.save()).toThrow()

      db.delete()
    })
  })

  describe('delete', () => {
    it('deletes the document', () => {
      const db = createTestDatabase({ 'test-1': { hello: 'world' } })
      const doc = db.getDocument('test-1')!

      doc.delete()

      const doc2 = db.getDocument('test-1')
      expect(doc2).toBeNull()

      db.delete()
    })

    it('does not allow deleting a released document', () => {
      const db = createTestDatabase({ 'test-1': {} })
      const doc = db.getDocument('test-1')!

      doc.release()

      expect(() => doc.delete()).toThrowError('Cannot delete a released document')

      db.delete()
    })
  })

  describe('release', () => {
    it('does not allow releasing a document that is already released', () => {
      const db = createTestDatabase({ 'test-1': {} })
      const doc = db.getDocument('test-1')!

      doc.release()

      expect(() => doc.release()).toThrowError('Cannot release a released document')

      db.delete()
    })
  })
})

describe('MutableDocument', () => {
  describe('create', () => {
    it('creates a new document with an auto-generated ID', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      expect(doc.value).toEqual({})
      expect(doc.id).toMatch(/^~/)

      doc.release()
      db.delete()
    })

    it('creates a new document with a custom ID', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db, 'custom-id')

      expect(doc.value).toEqual({})
      expect(doc.id).toBe('custom-id')

      doc.release()
      db.delete()
    })
  })

  describe('get id', () => {
    it('does not allow getting the id of a released document', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.release()

      expect(() => doc.id).toThrowError('Cannot get ID of a released document')
      db.delete()
    })
  })

  describe('get value', () => {
    it('does not allow getting data from a released document', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.release()

      expect(() => doc.value).toThrowError('Cannot get value of a released document')
      db.delete()
    })
  })

  describe('set value', () => {
    it('sets the doc\'s value', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.value = { testing: true }

      expect(doc.value).toEqual({ testing: true })

      doc.release()
      db.delete()
    })

    it('does not allow setting data on a released document', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.release()

      expect(() => { doc.value = { testing: true } }).toThrowError('Cannot set value of a released document')
      db.delete()
    })
  })

  describe('save', () => {
    it('persists the document', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db, 'saved_doc')

      doc.value = { testing: true }
      expect(doc.saved).toBe(false)
      doc.save()
      expect(doc.saved).toBe(true)
      doc.release()

      const doc2 = db.getMutableDocument('saved_doc')!
      expect(doc2.value).toEqual({ testing: true })
      doc2.release()

      db.delete()
    })

    it('does not allow saving a released document', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.release()

      expect(() => doc.save()).toThrowError('Cannot save a released document')

      db.delete()
    })
  })

  describe('delete', () => {
    it('deletes the document', () => {
      const db = createTestDatabase({ 'test-1': { hello: 'world' } })
      const doc1 = db.getMutableDocument('test-1')!

      doc1.delete()

      const doc2 = db.getMutableDocument('test-1')

      expect(doc2?.value).toEqual({})

      db.delete()
    })

    it('throws an error when trying to delete an unsaved document', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.value = { testing: true }

      expect(() => doc.delete()).toThrowError('Cannot delete an unsaved document')

      doc.release()
      db.delete()
    })

    it('does not allow deleting a released document', () => {
      const db = createTestDatabase({ 'test-1': {} })
      const doc = db.getDocument('test-1')!

      doc.release()

      expect(() => doc.delete()).toThrowError('Cannot delete a released document')

      db.delete()
    })
  })

  describe('release', () => {
    it('does not allow releasing a document that is already released', () => {
      const db = createTestDatabase()
      const doc = MutableDocument.create(db)

      doc.release()

      expect(() => doc.release()).toThrowError('Cannot release a released document')

      db.delete()
    })
  })
})
