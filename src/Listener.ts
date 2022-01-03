import { CBL } from './CBL'

export function removeListener(listenerToken: unknown) {
  return CBL.Listener_Remove(listenerToken)
}
