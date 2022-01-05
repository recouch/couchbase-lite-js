import { nanoid } from 'nanoid'
import { Database } from './Database'
import { MutableDocument } from './Document'

export function createTestDatabase(initDocs: { [id:string]: Record<string, unknown> } = {}) {
  const name = `tmp-db-${nanoid()}`

  const db = Database.open(name)

  Object.entries(initDocs).forEach(([id, data]) => {
    const doc = MutableDocument.create(db, id)

    doc.value = data
    doc.save()
    doc.release()
  })

  return db
}
