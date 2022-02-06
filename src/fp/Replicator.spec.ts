import { createServer } from 'http'
import { createTestDatabase } from './test-util'
import { createReplicator, replicatorConfiguration, replicatorStatus, startReplicator, stopReplicator } from './Replicator'
import { databaseName } from './Database'

describe('Replicator', () => {
  it('configures the replicator', () => {
    const { cleanup, db, dbName } = createTestDatabase()
    const replicator = createReplicator({ database: db, endpoint: 'ws://localhost:4984/db1' })

    expect(replicator).toBeTruthy()

    const config = replicatorConfiguration(replicator)

    expect(databaseName(config.database)).toBe(dbName)
    expect(replicatorStatus(replicator).activity).toBe('stopped')

    cleanup()
  })

  it('tries to connect to a URL endpoint', () =>
    new Promise<void>(resolve => {
      const { cleanup, db } = createTestDatabase()
      const replicator = createReplicator({ database: db, endpoint: 'ws://localhost:4984/db2' })
      const server = createServer(req => {
        stopReplicator(replicator)

        cleanup()
        server.close()

        expect(req).toBeTruthy()

        expect(replicatorStatus(replicator).activity).toBe('stopped')
        expect(replicatorStatus(replicator).progress.complete).toBe(1)

        resolve()
      })

      server.listen(4984)
      startReplicator(replicator)

      expect(replicatorStatus(replicator).activity).toBe('connecting')
    })
  )
})
