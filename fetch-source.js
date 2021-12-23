import crypto from 'crypto'
import extractZip from 'extract-zip'
import fs from 'fs'
import got from 'got'
import os from 'os'
import path from 'path'
import { exit, stdout } from 'process'
import stream from 'stream'
import { fileURLToPath } from 'url'
import { promisify } from 'util'
import { downloadUrls, shaUrls } from './sources.js'

const pipeline = promisify(stream.pipeline)

/**
 * @type string | undefined
 */
const downloadUrl = downloadUrls[os.platform()]

/**
 * @type string | undefined
 */
const shaUrl = shaUrls[os.platform()]

const dirname = path.dirname(fileURLToPath(import.meta.url))
const downloadDirectory = path.join(dirname, 'downloads')
const libDirectory = path.join(dirname, 'lib')

if (downloadUrl) {
  const filename = path.basename(downloadUrl)
  const filePath = path.join(downloadDirectory, filename)

  stdout.write(`Downloading ${downloadUrl}...\n`)
  if (!fs.existsSync(downloadDirectory)) fs.mkdirSync(downloadDirectory)
  await pipeline(got.stream(downloadUrl), fs.createWriteStream(filePath))

  stdout.write(`Downloading checksum from ${shaUrl}...\n`)
  const expectedSha = await got(shaUrl).then((res) => res.body.trim())

  stdout.write(`Verifying checksum ${expectedSha}...\n`)
  const sha = crypto.createHash('sha256')
  await pipeline(fs.createReadStream(filePath), sha)
  const actualSha = `${sha.digest('hex')}  ${filename}`
  const validChecksum = actualSha === expectedSha

  if (!validChecksum) {
    stdout.write('Invalid checksum\n')

    exit(1)
  }

  stdout.write('Extracting...\n')
  if (!fs.existsSync(libDirectory)) fs.mkdirSync(libDirectory)
  await extractZip(filePath, { dir: libDirectory })

  stdout.write('Done!\n')
}
