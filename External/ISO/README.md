
# ISO Packaging

## Prerequisites
To package GameCube and Wii ISOs, place the following files in this directory

- (Linux) dollz3
- (Windows) dollz3.exe
- (Windows) mkisofs.exe
- (Windows) cygwin1.dll
- gbi.hdr

On Linux, install mkisofs
- `sudo apt install mkisofs`

## Packaging
Simply package for `GameCube` or `Wii` (e.g. File->Package->GameCube).
ISO files will not be generated when packaging an "Embedded" version.
