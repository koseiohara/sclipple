# sclipple

A small command-line note manager based on keyword-addressed notes.

When working in a terminal, you often want to write something down quickly without creating files in the current directory.

sclipple stores notes in a dedicated location and lets you access them from anywhere using short keywords.

```sh
$ sclipple add todo
$ sclipple todo
```

The note can later be opened from any directory.

---

## Features

- Keyword-based note management
- Notes accessible from any directory
- Editor-independent workflow
- Plain text storage
- Fast full-text search using POSIX extended regular expressions
- Git integration for version control

---

## Installation

```sh
$ ./configure
$ make
$ make install
```

---

## Quick Start

Create a note:

```sh
$ sclipple add todo
```

Edit the note:

```sh
$ sclipple todo
```

Show the note:

```sh
$ sclipple show todo
```

List all notes:

```sh
$ sclipple ls
```

Search notes:

```sh
$ sclipple search deadline
```

Rename a note:

```sh
$ sclipple mv todo tasks
```

Remove a note:

```sh
$ sclipple rm tasks
```

---

## Concepts

Each note is identified by a unique **KEY**.

Example:

```sh
$ sclipple add project
```

creates a note associated with the key:

```text
project
```

The key is later used to edit, display, search, rename, or remove the note.

---

## Commands

### Create notes

```sh
$ sclipple add <KEY> [<KEY> ...]
```

Create one or more notes.

---

### Edit notes

```sh
$ sclipple <KEY> [<KEY> ...]
```

Open notes in the configured editor.

---

### List notes

```sh
$ sclipple ls [<KEY> ...]
```

List notes.

For each note, the output includes:

- KEY
- creation timestamp
- first non-empty line

---

### Show notes

```sh
$ sclipple show [<KEY> ...]
```

Print note contents.

Without arguments, all notes are displayed.

---

### Search notes

```sh
$ sclipple search <PATTERN> [<KEY> ...]
```

Search note contents using POSIX extended regular expressions.

Features:

- case-insensitive search
- optional note filtering by KEY
- colored output on terminals

Examples:

```sh
$ sclipple search deadline
$ sclipple search 'todo|urgent'
$ sclipple search 'error.*log' project
```

---

### Rename notes

```sh
$ sclipple mv <OLD_KEY> <NEW_KEY>
```

Rename a note.

---

### Remove notes

```sh
$ sclipple rm <KEY> [<KEY> ...]
```

Remove notes.

---

### Git integration

Run Git commands inside the sclipple data directory.

```sh
$ sclipple git status
$ sclipple git init
$ sclipple git add .
$ sclipple git commit -m "update notes"
```

This makes it easy to keep notes under version control.

---

## Configuration

sclipple reads:

```text
~/.sclipplerc
```

Example:

```ini
editor = vim -p
extension = txt
```

### Supported options

| Option | Description | Default |
|----------|-------------|----------|
| editor | Editor command used when opening notes | `vim -p` |
| extension | Extension used for newly created notes | `txt` |

Notes:

- Lines beginning with `#` are treated as comments.
- Single and double quotes around values are removed.

Example:

```ini
editor = "nvim -p"
extension = md
```

---

## Storage

All data is stored under:

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

Generated filenames have the form:

```text
KEY--YYYY-MM-DD-hh-mm-ss.EXT
```

Example:

```text
todo--2025-07-01-12-34-56.txt
```

---

## KEY Rules

A KEY:

- must be unique
- must not be `.` or `..`
- may contain:
  - letters
  - digits
  - `_`
  - `-`

---

## Design Goals

sclipple is intended for notes that do not belong in the current working directory but should remain instantly accessible from the command line.

Goals:

- simple storage
- fast access
- editor independence
- command-line workflow
- version-control friendliness



