import { blobContent, blobContentType, blobCreateJson, blobDigest, blobEquals, blobLength, closeBlobReader, closeBlobWriter, readBlobReader, openBlobContentStream, blobProperties, createBlobWithData, createBlobWithStream, createBlobWriter, databaseGetBlob, databaseSaveBlob, writeBlobWriter, documentSetBlob, documentIsBlob, documentGetBlob } from './Blob'
import { createDocument, getDocument, saveDocument } from './Document'
import { createTestDatabase, timeout } from './test-util'

describe('Blob', () => {
  describe('blobEquals', () => {
    it('returns true when blobs are equal', () => {
      const blob1 = createBlobWithData('text/plain', Buffer.from('buffer'))
      const blob2 = createBlobWithData('text/plain', Buffer.from('buffer'))

      expect(blobEquals(blob1, blob2)).toBe(true)
    })

    it('returns false when blobs are unequal', () => {
      const blob1 = createBlobWithData('text/plain', Buffer.from('possum'))
      const blob2 = createBlobWithData('text/plain', Buffer.from('opossum'))

      expect(blobEquals(blob1, blob2)).toBe(false)
    })
  })

  describe('blobCreateJson', () => {
    it('retrieves the metadata from the blob as JSON', () => {
      const blob = createBlobWithData('text/plain', Buffer.from('so meta'))
      const props = blobCreateJson(blob)

      expect(JSON.parse(props)['@type']).toBe('blob')
      expect(JSON.parse(props).content_type).toBe('text/plain')
    })
  })

  describe('blobProperties', () => {
    it('retrieves the metadata from the blob', () => {
      const blob = createBlobWithData('text/plain', Buffer.from('so meta'))
      const props = blobProperties(blob)

      expect(props['@type']).toBe('blob')
      expect(props.content_type).toBe('text/plain')
    })
  })

  describe('databaseGetBlob/databaseSaveBlob', () => {
    it('saves and retrieves a blob not belonging to a document', () => {
      const { cleanup, db } = createTestDatabase()
      const blob = createBlobWithData('text/plain', Buffer.from('save me'))
      const props = blobProperties(blob)

      expect(true).toBe(true)

      expect(databaseSaveBlob(db, blob)).toBe(true)

      const retrievedBlob = databaseGetBlob(db, props)
      expect(blobEquals(retrievedBlob, blob)).toBe(true)

      const buffer = blobContent(retrievedBlob)
      expect(buffer.toString()).toBe('save me')

      cleanup()
    })
  })

  describe('blobContentType', () => {
    it('returns the blob\'s content type', () => {
      const blob = createBlobWithData('text/ordinary', Buffer.from('my content'))

      expect(blobContentType(blob)).toBe('text/ordinary')
    })
  })

  describe('blobLength', () => {
    it('returns the blob\'s content length', () => {
      const blob = createBlobWithData('text/ordinary', Buffer.from('my long content'))

      expect(blobLength(blob)).toBe(15)
    })
  })

  describe('blobContent', () => {
    it('returns the blob\'s content', () => {
      const blob = createBlobWithData('text/plain', Buffer.from('some contents'))

      expect(blobContent(blob).toString()).toBe('some contents')
    })
  })

  describe('blobDigest', () => {
    it('returns the blob\'s hashed content', () => {
      const blob = createBlobWithData('text/ordinary', Buffer.from('digestif`'))

      expect(blobDigest(blob)).toBe('sha1-albsf3kuyQ1sI6iYQFkpXnOZ3Z8=')
    })
  })

  describe('read stream', () => {
    it('creates a blob in pieces', () => {
      const { cleanup, db } = createTestDatabase()
      const blob = createBlobWithData('text/plain', Buffer.from('onetwothree'))
      databaseSaveBlob(db, blob)

      const stream = openBlobContentStream(blob)

      expect(readBlobReader(stream, 3).toString()).toBe('one')
      expect(readBlobReader(stream, 3).toString()).toBe('two')
      expect(readBlobReader(stream, 5).toString()).toBe('three')

      cleanup()
    })

    it('closes the stream', () => {
      const { cleanup, db } = createTestDatabase()
      const blob = createBlobWithData('text/plain', Buffer.from('willfail'))
      databaseSaveBlob(db, blob)

      const stream = openBlobContentStream(blob)

      expect(() => closeBlobReader(stream)).not.toThrow()
      // TODO: test that blob is closed

      cleanup()
    })
  })

  describe('write stream', () => {
    it('creates a blob in pieces', () => {
      const { cleanup, db } = createTestDatabase()
      const stream = createBlobWriter(db)

      expect(writeBlobWriter(stream, Buffer.from('three '))).toBe(true)
      expect(writeBlobWriter(stream, Buffer.from('easy '))).toBe(true)
      expect(writeBlobWriter(stream, Buffer.from('pieces'))).toBe(true)

      const blob = createBlobWithStream('text/plain', stream)

      expect(databaseSaveBlob(db, blob)).toBe(true)
      expect(blobContent(blob).toString()).toBe('three easy pieces')

      cleanup()
    })

    it('closes the stream', () => {
      const { cleanup, db } = createTestDatabase()
      const stream = createBlobWriter(db)

      expect(() => closeBlobWriter(stream)).not.toThrow()
      // TODO: test that blob is closed

      cleanup()
    })
  })

  describe('documentSetBlob/documentGetBlob', () => {
    it('sets and gets the blob from a document', async () => {
      const { cleanup, db } = createTestDatabase()

      // Use closure to have doc and blob released automatically
      ;(() => {
        const doc = createDocument('attached')
        const blob = createBlobWithData('text/plain', Buffer.from('documentation2'))

        documentSetBlob(doc, 'attachment', blob)

        saveDocument(db, doc)
      })()

      // Give Node enough time to collect garbage
      await timeout(10)

      const doc2 = getDocument(db, 'attached')!
      expect(documentIsBlob(doc2, 'attachment')).toBe(true)
      const blob2 = documentGetBlob(doc2, 'attachment')

      expect(blobContent(blob2).toString()).toBe('documentation2')

      cleanup()
    })
  })
})
