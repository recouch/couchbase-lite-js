import { MutableDocument } from './Document'
import { Query } from './Query'
import { createTestDatabase } from './test-util'

describe('Query', () => {
  describe('create', () => {
    it('creates a query using JSON structure', () => {
      const db = createTestDatabase()
      const query = Query.create(db, ['select', { what: ['*'] }])

      expect(query).toBeInstanceOf(Query)

      query.release()
      db.delete()
    })

    it('creates a query using N1QL structure', () => {
      const db = createTestDatabase()
      const query = Query.create(db, 'SELECT * from _')

      expect(query).toBeInstanceOf(Query)

      query.release()
      db.delete()
    })
  })

  describe('execute', () => {
    it('executes a query using JSON structure', () => {
      const db = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = Query.create(db, ['SELECT', { WHAT: [['.'], ['.', '_id']], FROM: [{ as: 'value' }] }])

      const results = query.execute()

      expect(results).toHaveLength(2)
      expect(results).toContainEqual({ id: 'doc1', value: { name: 'Fiona' } })
      expect(results).toContainEqual({ id: 'doc2', value: { name: 'Milo' } })

      query.release()
      db.delete()
    })

    it('executes a query using N1QL structure', () => {
      const db = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = Query.create(db, 'SELECT _id, * FROM _ AS value')

      const results = query.execute()

      expect(results).toHaveLength(2)
      expect(results).toContainEqual({ id: 'doc1', value: { name: 'Fiona' } })
      expect(results).toContainEqual({ id: 'doc2', value: { name: 'Milo' } })

      query.release()
      db.delete()
    })

    it('does not allow operation on a released query', () => {
      const db = createTestDatabase()
      const query = Query.create(db, 'SELECT * FROM _')

      query.release()

      expect(() => query.execute()).toThrowError('Cannot execute a released query')

      db.delete()
    })
  })

  describe('explain', () => {
    it('explains a query', () => {
      const db = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const query = Query.create(db, 'SELECT _id, * FROM _ AS value')

      const explanation = query.explain()

      expect(typeof explanation).toBe('string')
      expect(explanation).toContain('AS value')

      query.release()
      db.delete()
    })

    it('does not allow operation on a released query', () => {
      const db = createTestDatabase()
      const query = Query.create(db, 'SELECT * FROM _')

      query.release()

      expect(() => query.explain()).toThrowError('Cannot explain a released query')

      db.delete()
    })
  })

  describe('addChangeListener', () => {
    it('listens to changes on a query', async () => {
      const timeout = () => new Promise(resolve => setTimeout(resolve, 300))
      const db = createTestDatabase({ doc1: { type: 'parent', name: 'Mom' }, doc2: { type: 'child', name: 'Milo' } })
      const query = Query.create(db, 'SELECT _id, * FROM _ AS value WHERE type == "child"')
      const cb = jest.fn()
      const stop = query.addChangeListener(cb)

      await timeout()
      expect(cb).toHaveBeenCalledTimes(1)
      expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }])

      cb.mockClear()

      const doc3 = MutableDocument.create(db, 'doc3')
      doc3.value = { type: 'parent', name: 'Dad' }
      doc3.save()
      await timeout()
      expect(cb).not.toHaveBeenCalled()

      const doc4 = MutableDocument.create(db, 'doc4')
      doc4.value = { type: 'child', name: 'Fiona' }
      doc4.save()
      await timeout()
      expect(cb).toHaveBeenCalledTimes(1)
      expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }, { id: 'doc4', value: { type: 'child', name: 'Fiona' } }])

      cb.mockClear()

      doc4.value = { type: 'nephew', name: 'Becky' }
      doc4.save()
      await timeout()
      expect(cb).toHaveBeenCalledTimes(1)
      expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }])

      stop()
      query.release()
      db.delete()
    })

    it('does not allow operation on a released query', () => {
      const db = createTestDatabase()
      const query = Query.create(db, 'SELECT * FROM _')
      const cb = jest.fn()

      query.release()

      expect(() => query.addChangeListener(cb)).toThrowError('Cannot listen to changes on a released query')

      db.delete()
    })

    describe('stop', () => {
      it('stops listens to changes on a query', async () => {
        const timeout = () => new Promise(resolve => setTimeout(resolve, 300))
        const db = createTestDatabase({ doc1: { type: 'parent', name: 'Mom' }, doc2: { type: 'child', name: 'Milo' } })
        const query = Query.create(db, 'SELECT _id, * FROM _ AS value WHERE type == "child"')
        const cb = jest.fn()
        const stop = query.addChangeListener(cb)

        await timeout()
        expect(cb).toHaveBeenCalledTimes(1)
        expect(cb).toHaveBeenCalledWith([{ id: 'doc2', value: { type: 'child', name: 'Milo' } }])

        cb.mockClear()
        stop()

        const doc3 = MutableDocument.create(db, 'doc3')
        doc3.value = { type: 'child', name: 'Fiona' }
        doc3.save()
        await timeout()
        expect(cb).not.toHaveBeenCalled()

        query.release()
        db.delete()
      }, 3000)
    })
  })

  describe('release', () => {
    it('does not allow releasing a query that is already released', () => {
      const db = createTestDatabase()
      const query = Query.create(db, 'SELECT * FROM _')

      query.release()

      expect(() => query.release()).toThrowError('Cannot release a released query')

      db.delete()
    })
  })
})
