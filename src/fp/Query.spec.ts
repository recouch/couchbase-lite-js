import {
  addQueryChangeListener,
  CBLJSONLanguage,
  CBLN1QLLanguage,
  createDocument,
  createQuery,
  executeQuery,
  explainQuery,
  getQueryParameters,
  saveDocument,
  setDocumentProperties,
  setQueryParameters
} from '../cblite'
import { createTestDatabase, timeout } from './test-util'

describe('query functions', () => {
  describe('addChangeListener', () => {
    it('listens to changes on query', async () => {
      const { cleanup, db } = createTestDatabase({ doc1: { type: 'parent', name: 'Mom' }, doc2: { type: 'child', name: 'Milo' } })
      const query = createQuery(db, 'SELECT _id, * FROM _ AS value WHERE type == "child"')
      const cb = jest.fn()
      const stop = addQueryChangeListener(query, cb)

      await timeout(500)
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

    it('throws on an invalid N1QL query', () => {
      const { cleanup, db } = createTestDatabase()

      expect(() => createQuery(db, 'SELECT * FROM *')).toThrowError(expect.objectContaining({ message: expect.stringContaining('N1QL syntax error') }))

      cleanup()
    })
  })

  describe('execute', () => {
    it('executes a JSON query', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = createQuery(db, CBLJSONLanguage, '["SELECT", { "WHAT": [["."], [".", "_id"]], "FROM": [{ as: "value" }] }]')

      const results = executeQuery(query)

      expect(results).toHaveLength(2)
      expect(results).toContainEqual({ id: 'doc1', value: { name: 'Fiona' } })
      expect(results).toContainEqual({ id: 'doc2', value: { name: 'Milo' } })

      cleanup()
    })

    it('executes a JSON query when passing an object as the 2nd param', () => {
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
      const query = createQuery(db, CBLN1QLLanguage, 'SELECT _id, * FROM _ AS value')

      const results = executeQuery(query)

      expect(results).toHaveLength(2)
      expect(results).toContainEqual({ id: 'doc1', value: { name: 'Fiona' } })
      expect(results).toContainEqual({ id: 'doc2', value: { name: 'Milo' } })

      cleanup()
    })

    it('executes a N1QL query when passing a string as the 2nd param', () => {
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

  describe('getQueryParameters', () => {
    it('sets the parameters of a query', () => {
      const { cleanup, db } = createTestDatabase()
      const query = createQuery<{ id: string }>(db, 'SELECT _id FROM _ WHERE name = $name')

      setQueryParameters(query, { name: 'Milo' })

      expect(getQueryParameters(query)).toEqual({ name: 'Milo' })

      cleanup()
    })
  })

  describe('setQueryParameters', () => {
    it('sets the parameters of a query', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = createQuery<{ id: string }>(db, 'SELECT _id FROM _ WHERE name = $name')

      setQueryParameters(query, { name: 'Milo' })

      const results1 = executeQuery(query)
      expect(results1).toHaveLength(1)
      expect(results1[0].id).toBe('doc2')

      setQueryParameters(query, { name: 'Fiona' })

      const results2 = executeQuery(query)
      expect(results2).toHaveLength(1)
      expect(results2[0].id).toBe('doc1')

      setQueryParameters(query, { name: 'Phil' })

      const results3 = executeQuery(query)
      expect(results3).toHaveLength(0)

      cleanup()
    })
  })
})
