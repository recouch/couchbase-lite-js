import { createDocument, saveDocument, setDocumentProperties } from './Document'
import { addQueryChangeListener, createQuery, executeQuery, explainQuery } from './Query'
import { createTestDatabase, timeout } from './test-util'

describe('query functions', () => {
  describe('addChangeListener', () => {
    it('listens to changes on query', async () => {
      const { cleanup, db } = createTestDatabase({ doc1: { type: 'parent', name: 'Mom' }, doc2: { type: 'child', name: 'Milo' } })
      const query = createQuery(db, 'SELECT _id, * FROM _ AS value WHERE type == "child"')
      const cb = jest.fn()
      const stop = addQueryChangeListener(query, cb)

      await timeout()
      expect(cb).toHaveBeenCalledTimes(1)
      expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }])

      cb.mockClear()

      const doc3 = createDocument('doc3')
      setDocumentProperties(doc3, { type: 'parent', name: 'Dad' })
      saveDocument(db, doc3)
      await timeout(500)
      expect(cb).not.toHaveBeenCalled()

      const doc4 = createDocument('doc4')
      setDocumentProperties(doc4, { type: 'child', name: 'Fiona' })
      saveDocument(db, doc4)
      await timeout(500)
      expect(cb).toHaveBeenCalledTimes(1)
      expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }, { id: 'doc4', value: { type: 'child', name: 'Fiona' } }])

      cb.mockClear()

      setDocumentProperties(doc4, { type: 'nephew', name: 'Becky' })
      saveDocument(db, doc4)
      await timeout(500)
      expect(cb).toHaveBeenCalledTimes(1)
      expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }])

      stop()
      cleanup()
    })

    describe('stop', () => {
      it('stops listens to changes on a query', async () => {
        const { cleanup, db } = createTestDatabase({ doc1: { type: 'parent', name: 'Mom' }, doc2: { type: 'child', name: 'Milo' } })
        const query = createQuery(db, 'SELECT _id, * FROM _ AS value WHERE type == "child"')
        const cb = jest.fn()
        const stop = addQueryChangeListener(query, cb)

        await timeout(500)
        expect(cb).toHaveBeenCalledTimes(1)
        expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }])

        cb.mockClear()
        stop()

        const doc3 = createDocument('doc3')
        setDocumentProperties(doc3, { type: 'child', name: 'Fiona' })
        saveDocument(db, doc3)
        await timeout(500)
        expect(cb).not.toHaveBeenCalled()

        cleanup()
      })
    })
  })

  describe('createQuery', () => {
    it('creates a JSON query', () => {
      const { cleanup, db } = createTestDatabase()
      const query = createQuery(db, ['select', { what: ['*'] }])

      expect(query).toBeTruthy()

      cleanup()
    })

    it('creates a N1QL query', () => {
      const { cleanup, db } = createTestDatabase()
      const query = createQuery(db, 'SELECT * FROM _')

      expect(query).toBeTruthy()

      cleanup()
    })
  })

  describe('execute', () => {
    it('executes a JSON query', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = createQuery(db, ['SELECT', { WHAT: [['.'], ['.', '_id']], FROM: [{ as: 'value' }] }])

      const results = executeQuery(query)

      expect(results).toHaveLength(2)
      expect(results).toContainEqual({ id: 'doc1', value: { name: 'Fiona' } })
      expect(results).toContainEqual({ id: 'doc2', value: { name: 'Milo' } })

      cleanup()
    })

    it('executes a N1QL query', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = createQuery(db, 'SELECT _id, * FROM _ AS value')

      const results = executeQuery(query)

      expect(results).toHaveLength(2)
      expect(results).toContainEqual({ id: 'doc1', value: { name: 'Fiona' } })
      expect(results).toContainEqual({ id: 'doc2', value: { name: 'Milo' } })

      cleanup()
    })
  })

  describe('explain', () => {
    it('explains a query', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = createQuery(db, 'SELECT _id, * FROM _ AS value')

      const explanation = explainQuery(query)
      expect(typeof explanation).toBe('string')
      expect(explanation).toContain('AS value')

      cleanup()
    })
  })
})
