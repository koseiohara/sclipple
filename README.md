# sclipple

A lightweight note manager for people who live in the terminal.

sclipple is a combination of clip and scribble.

The idea is simple:

- keep small notes
- access them by a short key
- edit them with your favorite editor
- store everything as plain text

No database.
No background service.
No proprietary format.

Just text files.

---

## Why?

Many note-taking tools are optimized for large documents, graphical interfaces, or cloud synchronization.

Sometimes you only need a quick place to store things like:

- commands you use repeatedly
- snippets of text
- project notes
- temporary references
- personal memos

sclipple provides a minimal interface for managing such notes directly from the command line.

---

## Features

- Key-based note management
- Plain text storage
- Editor integration
- Regular-expression search
- Git integration
- Human-readable data layout

---

## Installation

Build from source:

bash make sudo make install 

---

## Quick Start

Create a note:

bash sclipple add todo 

Edit it:

bash sclipple todo 

Show its contents:

bash sclipple show todo 

Search all notes:

bash sclipple search "important" 

List available notes:

bash sclipple ls 

---

## Concept

Every note is identified by a unique key.

text <KEY> 

The key acts as both an identifier and a shortcut for opening the note.

For example:

bash sclipple project 

opens the note associated with the key:

text project 

This makes frequently used notes easy to access from the shell.

---

## Command Reference

### Create notes

bash sclipple add <KEY> [<KEY> ...] 

Create one or more notes.

### Edit notes

bash sclipple <KEY> [<KEY> ...] 

Open notes in the configured editor.

### Show note contents

bash sclipple show <KEY> [<KEY> ...] 

Display note contents.

### List notes

bash sclipple ls [<KEY> ...] 

List registered notes.

### Search notes

bash sclipple search <PATTERN> [<KEY> ...] 

Search note contents using a regular expression.

### Rename notes

bash sclipple mv <OLD_KEY> <NEW_KEY> 

Rename a note.

### Remove notes

bash sclipple rm <KEY> [<KEY> ...] 

Delete notes.

### Git integration

bash sclipple git <ARGS...> 

Execute git commands inside the sclipple data directory.

Examples:

bash sclipple git status sclipple git log sclipple git commit 

---

## Configuration

Configuration file:

text ~/.sclipplerc 

Example:

ini editor=nvim 

---

## Data Storage

By default, data is stored in:

text ~/.sclipple/ 

Typical layout:

text ~/.sclipple/ ├── notes/ ├── .list.csv └── .git/ 

Since notes are stored as plain text files, they can be inspected, backed up, synchronized, or version-controlled using standard tools.

---

## Help

General help:

bash sclipple help 

Command-specific help:

bash sclipple help add sclipple help rm sclipple help mv sclipple help ls sclipple help show sclipple help search sclipple help git 

---

## License

See LICENSE for details.
