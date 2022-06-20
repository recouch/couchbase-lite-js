# Couchbase Lite for JS

Node.js bindings for Couchbase Lite.

## Disclaimer
> This library is NOT SUPPORTED BY COUCHBASE. Even if you are a Couchbase customer, our otherwise awesome support team cannot help you with using this library.

## Install

```tsbash
npm install --save @recouch/couchbase-lite
```

## Usage

```ts
import {
  closeDatabase,
  getDocument,
  getDocumentProperties,
  openDatabase
} from '@recouch/couchbase-lite'

const db = openDatabase('my-database')
const doc = getDocument(db, 'my-document')

console.log(getDocumentProperties(doc))

closeDatabase(db)
```

## API

### addDatabaseChangeListener
#### Parameters
- `database` **DatabaseRef**
- `handler` Event handler `(docIDs: string[]) => void`

#### Returns
A function that removes the change listener

```ts
const stop = addDatabaseChangeListener(db, (docIDs =>
  docIDs.forEach(docID => console.log(`${docID} changed`))
))

stop()
```

### beginTransaction
#### Parameters
- `database` **DatabaseRef**

```ts
beginTransaction(db)
```

### closeDatabase(database: DatabaseRef): boolean
#### Parameters
- `database` **DatabaseRef**

```ts
beginTransaction(db)
```

### deleteDatabase
Delete a database that was opened with `openDatabase()`

#### Parameters
- `database` **DatabaseRef**

#### Returns
**boolean** will be true if database was successfully deleted

```ts
deleteDatabase(db)
```

### deleteDatabase alternative version
Delete a database that has not been opened.

#### Parameters
- `name` **string** Database name
- `directory` **string** Path to database location

#### Returns
**boolean** will be true if database was successfully deleted

```ts
deleteDatabase('my-unnecessary-database', 'path/to/db/dir')
```

### endTransaction
#### Parameters
- `database` **DatabaseRef**
- `commit` **boolean**

```ts
beginTransaction(db)

// do stuff ...

endTransaction(db, true)
```

### abortTransaction
Shortcut for `endTransaction(db, true)`

#### Parameters
- `database` **DatabaseRef**

```ts
beginTransaction(db)

// do stuff ...

abortTransaction(db)
```


### commitTransaction
Shortcut for `endTransaction(db, false)`

#### Parameters
- `database` **DatabaseRef**

```ts
beginTransaction(db)

// do stuff ...

commitTransaction(db)
```

### openDatabase
#### Parameters
- `name` **string** Database name
- `directory` (optional) **string** Path to database location

#### Returns
**DatabaseRef** to be passed into other database operations

```ts
const db = openDatabase('my-database', 'path/to/db/dir')
```

### databaseName
#### Parameters
- `database` **DatabaseRef**

#### Returns
**string** name of the database

```ts
const name = databaseName(db)
```

### databasePath
#### Parameters
- `database` **DatabaseRef**

#### Returns
**string** path to the database

```ts
const path = databasePath(db)
```
