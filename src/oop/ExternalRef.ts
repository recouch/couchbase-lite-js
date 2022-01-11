declare const External: unique symbol

export type BasicValue = Record<string, unknown>
export type ExternalRef<Type extends string, T extends BasicValue = BasicValue> = { readonly [External]: Type, readonly value: T }
