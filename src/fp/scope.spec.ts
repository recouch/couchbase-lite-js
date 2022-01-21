import { scopeDatabase } from './scope'
import { createTestDatabase } from './test-util'

describe('scopeDatabase', () => {
  describe('createDocument', () => {
    it('returns mutable document methods scoped to the parent database and a new document', () => {
      const { cleanup, db } = createTestDatabase()
      const sdb = scopeDatabase(db)
      const doc = sdb.createDocument('doc1')

      expect(doc?.getID()).toBe('doc1')
      expect(doc?.setProperties({ name: 'Fiona' })).toBe(true)
      expect(doc?.getProperties()).toEqual({ name: 'Fiona' })
      expect(doc?.save()).toBe(true)
      expect(sdb.getDocument('doc1')?.getProperties()).toEqual({ name: 'Fiona' })
      expect(doc?.delete()).toBe(true)
      expect(sdb.getDocument('doc1')).toBeNull()

      cleanup()
    })
  })

  describe('createQuery', () => {
    it('returns query methods scoped to the parent database and query', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const sdb = scopeDatabase(db)
      const query = sdb.createQuery<{ name: string }, { id: string }>(`
        SELECT *
        FROM _ WHERE _id = $id
      `)

      expect(query.setParameters({ id: 'doc2' })).toBe(true)
      expect(query.getParameters()).toEqual({ id: 'doc2' })
      expect(query.execute()).toEqual([{ _: { name: 'Milo' } }])

      cleanup()
    })
  })

  describe('getDocument', () => {
    it('returns document methods scoped to the parent database and document', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const sdb = scopeDatabase(db)
      const doc = sdb.getDocument('doc1')

      expect(doc?.getID()).toBe('doc1')
      expect(doc?.getProperties()).toEqual({ name: 'Fiona' })
      expect(doc?.delete()).toBe(true)
      expect(sdb.getDocument('doc1')).toBeNull()

      cleanup()
    })
  })

  describe('getMutableDocument', () => {
    it('returns mutable document methods scoped to the parent database and document', () => {
      const { cleanup, db } = createTestDatabase({ doc1: { name: 'Fiona' }, doc2: { name: 'Milo' } })
      const sdb = scopeDatabase(db)
      const doc = sdb.getMutableDocument('doc1')

      expect(doc?.getID()).toBe('doc1')
      expect(doc?.getProperties()).toEqual({ name: 'Fiona' })
      expect(doc?.setProperties({ name: 'Vienna' })).toBe(true)
      expect(doc?.getProperties()).toEqual({ name: 'Vienna' })
      expect(doc?.save()).toBe(true)
      expect(sdb.getDocument('doc1')?.getProperties()).toEqual({ name: 'Vienna' })
      expect(doc?.delete()).toBe(true)
      expect(sdb.getDocument('doc1')).toBeNull()

      cleanup()
    })
  })
})
