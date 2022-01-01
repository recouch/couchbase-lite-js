import fs from 'fs'
import { join } from 'path'
import { Database } from './Database'

describe('Database', () => {
  it('creates a new database on disk', () => {
    const dbName = 'tmp_db'
    const dbPath = join(__dirname, `../${dbName}.cblite2`)

    // Ensure db does not yet exist
    fs.existsSync(dbPath) && fs.rmSync(dbPath, { recursive: true })

    const db = Database.open(dbName)

    expect(db).toBeTruthy()
    expect(fs.statSync(dbPath).isDirectory()).toBe(true)
    expect(db.close()).toBe(true)

    fs.rmSync(dbPath, { recursive: true })
  })

  it('opens an existing database', () => {
    const dbName = 'existing_db'
    const dbPath = join(__dirname, `../${dbName}.cblite2`)

    // Ensure db exists
    if (!fs.existsSync(dbPath)) {
      Database.open(dbName).close()
    }

    const db = Database.open(dbName)

    expect(db).toBeTruthy()
    expect(db.close()).toBe(true)
    
    fs.rmSync(dbPath, { recursive: true })
  })
})
