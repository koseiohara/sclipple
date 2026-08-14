# sclipple @VERSION@

sclipple is a small command-line note manager based on keyword-addressed notes.
It stores plain-text notes in a dedicated location and makes them accessible from any directory using short keys.

```sh
sclipple add todo
sclipple todo
```

## Downloads

Choose the archive for your system:

| System | Asset |
|---|---|
| Linux x86_64 | `sclipple-@VERSION@-linux-x86_64.tar.gz` |
| Linux arm64 | `sclipple-@VERSION@-linux-arm64.tar.gz` |
| macOS Apple Silicon | `sclipple-@VERSION@-macos-arm64.tar.gz` |

The Linux binaries are built on Ubuntu 24.04. The macOS binary is built for Apple Silicon with a deployment target of macOS 12.0 or later.

`SHA256SUMS` contains checksums for all binary archives.

## Installation

Extract the archive and install the binary into a directory on your `PATH`:

```sh
tar -xzf sclipple-@VERSION@-<platform>.tar.gz
```

Replace `<platform>` with `linux-x86_64`, `linux-arm64`, or `macos-arm64`.

To build from source instead:

```sh
./configure
make
make check
make install
```

## Quick start

```sh
sclipple add todo       # Create a note
sclipple todo           # Edit it
sclipple ls             # List notes
sclipple show todo      # Print its contents
sclipple search urgent  # Search all notes
```

Configuration is read from `~/.sclipplerc`.
See the README for commands, configuration, storage layout, and Git integration.


