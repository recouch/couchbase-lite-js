import { partialRight } from 'lodash/fp'
import { endTransaction } from '../cblite'

export const abortTransaction = partialRight(endTransaction, [false])
export const commitTransaction = partialRight(endTransaction, [true])
