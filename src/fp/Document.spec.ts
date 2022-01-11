import { closeDatabase } from './Database'
import { addDocumentChangeListener, createDocument, deleteDocument, getDocument, getDocumentID, getDocumentProperties, getMutableDocument, saveDocument, setDocumentProperties } from './Document'
import { createTestDatabase } from './test-util'

describe('document functions', () => {
  describe('addDocumentChangeListener', () => {
    it('listens to document changes', async () => {
      const { cleanup, db } = createTestDatabase({ testDoc: { children: 1 } })
      const doc = getMutableDocument(db, 'testDoc')!
      const fn = jest.fn()
      const stop = addDocumentChangeListener(db, 'testDoc', fn)

      expect(typeof stop).toBe('function')

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).not.toHaveBeenCalled()

      setDocumentProperties(doc, { children: 2 })
      saveDocument(db, doc)

      await new Promise(resolve => setTimeout(resolve, 10))
      expect(fn).toHaveBeenCalledTimes(1)
      expect(fn).toHaveBeenCalledWith('testDoc')
      fn.mockClear()

      const doc2 = createDocument('testDoc2')
      setDocumentProperties(doc2, { children: 0 })
      saveDocument(db, doc2)
      expect(fn).not.toHaveBeenCalled()

      expect(stop()).toBe(true)

      cleanup()
    })
  })

  describe('createDocument', () => {
    it('creates a new document ref with a given ID', () => {
      const doc = createDocument('hello')

      expect(getDocumentID(doc)).toBe('hello')
    })

    it('creates a new document ref with an auto-generated ID', () => {
      const doc = createDocument()

      expect(getDocumentID(doc)).toMatch(/^~/)
    })
  })

  describe('deleteDocument', () => {
    it('deletes a document', () => {
      const { cleanup, db } = createTestDatabase({ 'test-1': { hello: 'world' } })
      const doc1 = getMutableDocument(db, 'test-1')!

      deleteDocument(db, doc1)

      const doc2 = getMutableDocument(db, 'test-1')!

      expect(getDocumentProperties(doc2)).toEqual({})

      cleanup()
    })

    it('throws an error when trying to delete an unsaved document', () => {
      const { cleanup, db } = createTestDatabase()
      const doc = createDocument()

      setDocumentProperties(doc, { testing: true })

      expect(() => deleteDocument(db, doc)).toThrowError('Error deleting document')

      cleanup()
    })
  })

  describe('getDocument', () => {
    it('gets a non-mutable document by ID from the database', () => {
      const { cleanup, db } = createTestDatabase({ boy: { name: 'Milo' } })

      const retrievedDoc = getDocument<{ name: string }>(db, 'boy')
      expect(getDocumentProperties(retrievedDoc!).name).toBe('Milo')

      cleanup()
    })

    it('returns null when document is not found', () => {
      const { cleanup, db } = createTestDatabase({ boy: { name: 'Milo' } })

      const retrievedDoc = getDocument<{ name: string }>(db, 'girl')
      expect(retrievedDoc).toBeNull()

      cleanup()
    })

    it('does not allow operation on a closed database', () => {
      const { cleanup, db } = createTestDatabase({ boy: { name: 'Milo' } })

      closeDatabase(db)
      expect(() => getDocument(db, 'boy')).toThrowError('Database is closed')

      cleanup()
    })
  })

  describe('getDocumentID', () => {
    it('returns the ID of the document', () => {})
  })

  describe('getDocumentProperties', () => {
    it('returns the data of the document', () => {})
  })

  describe('getMutableDocument', () => {
    it('gets a mutable document by ID from the database', () => {})
  })

  describe('saveDocument', () => {
    it('saves a mutable document', () => {})
  })

  describe('setDocumentProperties', () => {
    it('sets the data of a mutable document', () => {})
  })
})
