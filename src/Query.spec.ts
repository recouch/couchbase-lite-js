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
