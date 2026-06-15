# sclipple

A command-line note manager.

Sometimes you want to write something down while working in a terminal.
Creating a file in the current directory is often the wrong choice. Switching to a dedicated notes directory is also unnecessary work.
sclipple stores notes in a single location and lets you access them from anywhere using short keys.
```sh
$ sclipple add ssh
$ sclipple ssh
```

A note created in one directory can be reopened later from any other directory.

## Installation

```sh
./configure
make
make install
```


## Example

Create a note:

```sh
sclipple add todo
```

Open it:

```sh
sclipple todo
```

List notes:

```sh
sclipple ls
```

Search notes:

```sh
sclipple search deadline
```

Display notes:

```sh
sclipple show todo
```

Rename a note:

```sh
sclipple mv todo tasks
```

Remove a note:

```sh
sclipple rm tasks
```

The location of a note does not depend on the current working directory.

## Commands

### Create notes

```sh
sclipple add <KEY> [<KEY> ...]
```

Create one or more notes.

### Edit notes

```sh
sclipple <KEY> [<KEY> ...]
```

Open notes in the configured editor.

### List notes

```sh
sclipple ls [<KEY> ...]
```

List notes.

### Show notes

```sh
sclipple show <KEY> [<KEY> ...]
```

Print note contents.

### Search notes

```sh
sclipple search <PATTERN> [<KEY> ...]
```

Search notes using POSIX extended regular expressions.

### Rename notes

```sh
sclipple mv <OLD_KEY> <NEW_KEY>
```

Rename a note.

### Remove notes

```sh
sclipple rm <KEY> [<KEY> ...]
```

Remove notes.

### Git integration

```sh
sclipple git <ARGS...>
```

Execute Git commands in the sclipple data directory.

## Configuration

sclipple reads configuration from:

```text
~/.sclipplerc
```

Example:

```ini
editor = vim -p
extension = txt
```

### editor

Editor command used to open notes.

### extension

Default extension used when creating notes.

## Storage

By default, sclipple stores data under:

```text
~/.sclipple/
```

Typical layout:

```text
~/.sclipple/
├── notes/
└── .list.csv
```

Notes are stored as ordinary text files.

## Design

sclipple is intended for notes that do not belong in the current working directory but should still be easy to access from the command line.

The primary goals are:

- simple storage
- fast access
- editor independence
- command-line workflow

