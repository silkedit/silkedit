# SilkEdit

[![Build Status](https://travis-ci.org/silkedit/silkedit.svg?branch=master)](https://travis-ci.org/silkedit/silkedit)
[![Build status](https://ci.appveyor.com/api/projects/status/2not2atlf4v17r2s?svg=true)](https://ci.appveyor.com/project/shinichy/silkedit)

SilkEdit is the simple, modern, cross-platform text editor.

## How to build

### System requirements:

**MacOS**

- [Qt](https://www.qt.io/)
- [Node.js](https://nodejs.org/)
- [CMake](https://cmake.org/)
- [Conan](http://docs.conan.io/en/latest/installation.html)
- Xcode
- Command Line Tools for Xcode
- Ninja

Set QTDIR environment variable e.g. `export QTDIR=<Qt root directory>`

**Windows**

- [Qt](https://www.qt.io/)
- [Node.js](https://nodejs.org/)
- [CMake](https://cmake.org/)
- [Conan](http://docs.conan.io/en/latest/installation.html)
- Python
- [Msys2](https://sourceforge.net/projects/msys2/files/Base/)
- [Visual Studio 2015](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx?wt.mc_id=github_microsoft_vscode) or [Visual C++ Build Tools](http://landinghub.visualstudio.com/visual-cpp-build-tools)

Set QTDIR environment variable e.g. `set QTDIR=<Qt root directory>`

Click msys2_shell.bat and run these commands.

```bash
pacman -Suy
pacman -S wget
```

### Building an app

In windows, please run the following commands in msys2_shell.bat

```bash
git clone https://github.com/silkedit/silkedit.git
cd silkedit
./run resolve
npm run minify
./run build
```

After `./run build` finishes, the executable is created in `build` directory.
