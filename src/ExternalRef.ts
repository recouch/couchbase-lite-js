declare const External: unique symbol

export type ExternalRef<Type extends string> = { readonly [External]: Type }
