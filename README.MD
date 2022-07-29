# nina

The **n**on **i**nconvenient **n**uke **a**pplication is a universal data overwriting and destruction software.

## Features

- Overwrite data with zeros (read from *dev/zero*)
- Overwrite data with randomness (read from */dev/urandom*)
- Kernel caching is prevented using the fsync(2) system call which synchronizes the overwritten state with the storage device immediately

## Disclaimer

### Use this software at your own risk!

> This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY [...].

This means especially:  
**I am not responsible** if you use this program (wrongly) and thereby destroy
important files on your system.

## Build

`make`

## Usage


The feature set is *very* limited hence the name "non inconvenient".

General usage: `nina <file path> [--nuke] [--zero | --random]`

If no options are provided, the program asks interactively for your input.

To delete all files in a directory use: `find . -type f | xargs -n 1 nina --nuke <--zero | --random>`

Copyright notice: `nina --help`

## Example

```
$ nina test
File: test | Size: 314 Byte(s)
Nuke the file to nirvana? [yes/*]
yes
Choose method: [0] /dev/zero, [1] /dev/urandom
1
[ BEGIN ] write(2): Overwrite data using synchronous I/O
[SUCCESS] write(2): Overwrite data
[SUCCESS] fsync(2): Synchronize state with storage device
```
