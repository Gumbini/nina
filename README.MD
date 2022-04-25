# nina

The **n**on **i**nconvenient **n**uke **a**pplication is a universal data overwriting and destruction software.

## Features

- Overwrite data with zeros (read from *dev/zero*)
- Overwrite data with randomness (read from */dev/urandom*)
- Kernel caching is prevented using the fsync(2) system call which synchronizes the overwritten state with the storage device immediately

## Build

`make`

## Usage

The feature set is *very* limited thus the name "non inconvenient".

General usage: `nina <file path>`

Copyright notice: `nina --help`

## Example

```
$ nina test
File: test | Size: 314 Byte(s)
Proceed to nuke the file to nirvana? [yes/*]
yes
Choose method: [0] /dev/zero, [1] /dev/urandom
1
[SUCCESS] write: Overwrite data
[SUCCESS] fsync: Synchronize state with storage device
```
