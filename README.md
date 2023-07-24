# OMORI Patcher

## Installation

You can install omori-patcher either via the [gui installer](https://github.com/cemrk2/omori-autopatcher) or manually

#### Manual installation

- Create a dump of omori using [UWPDumper](https://github.com/Wunkolo/UWPDumper/releases/)
- Use a tool like [dnSpyEx](https://github.com/dnSpyEx/dnSpy/releases/) to make a copy of Chowdren.exe, rename it to `ChowdrenNoDyB.exe` and disable `Dynamic Base`
- Make a copy of `ChowdrenNoDyB.exe` called `OMORI.exe`
- Open `ChowdrenNoDyB.exe` in [x64dbg](https://x64dbg.com/) and apply the patches in `ChowdrenNoDyB.1337` to `OMORI.exe`

## Building

#### Requirements

- [Microsoft Visual Studio w/ MSVC](https://visualstudio.microsoft.com/downloads/)
- [Nim](https://nim-lang.org/install.html)

### Dependencies

Most dependencies can just be installed by running `nimble install`, however there is a dependency that must be installed manually

Clone the [jsonpatch.nim](https://github.com/hnicke/jsonpatch.nim) repo
Run `nimble install`

#### Building

To create a release build of omori-patcher run `nimble release`

#### Running from source

To run omori-patcher, create a patched copy of OMORI inside the OMORI directory, then run `nimble debug`