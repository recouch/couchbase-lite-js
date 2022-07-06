import { endTransaction } from '../cblite'
import { DatabaseRef } from '../types'

export const abortTransaction = (database: DatabaseRef) => endTransaction(database, false)
export const commitTransaction = (database: DatabaseRef) => endTransaction(database, true)
