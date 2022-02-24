import { createServer } from 'http'
import { createTestDatabase, timeout } from './test-util'
import { addDocumentReplicationListener, addReplicatorChangeListener, createReplicator, replicatorConfiguration, replicatorStatus, startReplicator, stopReplicator } from './Replicator'
import { databaseName } from './Database'
import { createDocument, saveDocument } from './Document'

function getRandomInt(min: number, max: number) {
  min = Math.ceil(min)
  max = Math.floor(max)
  return Math.floor(Math.random() * (max - min + 1)) + min
}

const getRandomPort = () => getRandomInt(49152, 65535)

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
      const port = getRandomPort()
      const replicator = createReplicator({ database: db, endpoint: `ws://localhost:${port}/db2` })
      const server = createServer(req => {
        stopReplicator(replicator)

        cleanup()
        server.close()

        expect(req).toBeTruthy()

        expect(replicatorStatus(replicator).activity).toBe('stopped')
        expect(replicatorStatus(replicator).progress.complete).toBe(1)

        resolve()
      })

      server.listen(port)
      startReplicator(replicator)

      expect(replicatorStatus(replicator).activity).toBe('connecting')
    })
  )

  describe('addReplicatorChangeListener', () => {
    it('reports status changes', async () => {
      const { cleanup, db } = createTestDatabase()
      const port = getRandomPort()
      const replicator = createReplicator({ database: db, endpoint: `ws://localhost:${port}/db3` })
      const replicatorChangeListener = jest.fn()
      const server = createServer().listen(port)

      addReplicatorChangeListener(replicator, replicatorChangeListener)
      startReplicator(replicator)

      await timeout(25)

      cleanup()
      server.close()

      await timeout(25)

      expect(replicatorChangeListener).toHaveBeenCalledWith(expect.objectContaining({ activity: 'stopped' }))
    })
  })
})
