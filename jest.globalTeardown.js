const fs = require('fs')
const { join } = require('path')

const directory = join(__dirname, 'test-output')

const cleanUpTestOutput = () =>
  fs.existsSync(directory) && fs.rmSync(directory, { recursive: true })

const teardown = () => {
  cleanUpTestOutput()
}

module.exports = teardown
