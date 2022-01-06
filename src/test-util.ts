import fs from 'fs'
import { join } from 'path'
import { nanoid } from 'nanoid'
import { Database } from './Database'
import { MutableDocument } from './Document'

export const testDirectory = join(__dirname, '../test-output')

export function createTestDatabase(initDocs: { [id: string]: Record<string, unknown> } = {}) {
  if (!fs.existsSync(testDirectory)) fs.mkdirSync(testDirectory)

  const name = `tmp-db-${nanoid()}`
  const db = Database.open(name, testDirectory)

  Object.entries(initDocs).forEach(([id, data]) => {
    const doc = MutableDocument.create(db, id)

    doc.value = data
    doc.save()
    doc.release()
  })

  return db
}

export const getDbPath = (db:Database) => join(testDirectory, `${db.name}.cblite2`)
