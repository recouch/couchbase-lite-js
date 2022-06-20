import { once } from 'node:events'
import { createServer } from 'node:http'
import { databaseName, addReplicatorChangeListener, createReplicator, replicatorConfiguration, replicatorStatus, startReplicator, stopReplicator } from '../cblite'
import { createTestDatabase, timeout } from './test-util'

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

  it('tries to connect to a URL endpoint', async () => {
    const { cleanup, db } = createTestDatabase()
    const port = getRandomPort()
    const replicator = createReplicator({ database: db, endpoint: `ws://localhost:${port}/db2` })
    const server = createServer((_req, res) => res.end()).listen(port)
    const handledRequest = once(server, 'request')

    startReplicator(replicator)

    await expect(handledRequest).resolves.toContainEqual(expect.objectContaining({ url: '/db2/_blipsync' }))

    stopReplicator(replicator)

    await new Promise(resolve => server.close(resolve))
    cleanup()
  })

  describe('addReplicatorChangeListener', () => {
    it('reports status changes', async () => {
      const { cleanup, db } = createTestDatabase()
      const port = getRandomPort()
      const replicator = createReplicator({ database: db, endpoint: `ws://localhost:${port}/db3` })
      const replicatorChangeListener = jest.fn()
      const server = createServer((_req, res) => res.end()).listen(port)

      server.on('*', evt => console.log('evt', evt))

      const stop = addReplicatorChangeListener(replicator, replicatorChangeListener)
      startReplicator(replicator)
      await timeout(50)

      stopReplicator(replicator)
      await timeout(50)

      expect(replicatorChangeListener).toHaveBeenCalledWith(expect.objectContaining({ activity: 'stopped' }))

      await new Promise(resolve => server.close(resolve))
      stop()
      cleanup()
    })
  })
})
